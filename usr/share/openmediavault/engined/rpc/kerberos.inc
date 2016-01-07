<?php

/**
 * Copyright (C) 2009-2013 Volker Theile <volker.theile@openmediavault.org>
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceKerberos extends OMVRpcServiceAbstract {
	private $principalRegex = "/(.*)\/(.*)@(.*)$/";

	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Kerberos";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this -> registerMethod("getSettings");
		$this -> registerMethod("setSettings");
		$this -> registerMethod("getKeyTab");
		$this -> registerMethod("addKeyFromKeyServer");
		$this -> registerMethod("removeKey");
		$this -> registerMethod("createKey");
	}

	/**
	 * Get the Kerberos configuration settings.
	 * @param params The method params
	 * @param context The context of the caller.
	 * @return A JSON object containing the fields required
	 *	to configure Kerberos.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;

		// Validate the RPC caller context.
		$this -> validateMethodContext($context, array(
				"role" => OMV_ROLE_ADMINISTRATOR
		));

		// Get the configuration object.
		$xpath = "//services/kerberos";
		$object = $xmlConfig -> get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED);
		}

		// Modify result object.
		if(array_key_exists('kdcs', $object) && array_key_exists('kdc', $object['kdcs'])) {
			if(is_array($object['kdcs']['kdc'])) {
				$object['kdcs'] = implode(',', $object['kdcs']['kdc']);
			} else {
				$object['kdcs'] = $object['kdcs']['kdc'];
			}
		}
		
		$object['enable'] = boolvalEx($object['enable']);
		$object['logging'] = boolvalEx($object['logging']);
		$object['nfs-enabled'] = boolvalEx($object['nfs-enabled']);
		$object['smb-enabled'] = boolvalEx($object['smb-enabled']);
		$object['ssh-enabled'] = boolvalEx($object['ssh-enabled']);
		$object['pam-enabled'] = boolvalEx($object['pam-enabled']);

		return $object;
	}

	/**
	 * Set the Kerberos configuration settings.
	 *
	 * @param params An array containing all of the fields and values.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setSettings($params, $context) {
		global $xmlConfig;

		// Validate the RPC caller context
		$this -> validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));

		// Validate the parameters of the RPC servicemethod.
		$this -> validateMethodParams($params, '{
			"type": "object",
			"properties": {
				"enable": {"type": "boolean"},
				"realm": {"type": "string"},
				"kdcs": {"type": "string"},
				"adminServer": {"type": "string"},
				"logging": {"type": "boolean"},
				"extraoptions": {"type": "string"},
				"nfs-enabled": {"type": "boolean"},
				"smb-enabled": {"type": "boolean"},
				"ssh-enabled": {"type": "boolean"},
				"pam-enabled": {"type": "boolean"}
			}
		}');

		// Prepare the configuration object.
		$kdcs = explode(trim($params['kdcs']));
		
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"realm" => strtoupper(trim($params['realm'])),
			"kdcs" => array(
				"kdc" => preg_split("/[,;]/", trim($params['kdcs']))
			),
			"adminServer" => trim($params['adminServer']),
			"logging" => array_boolval($params, 'logging'),
			"extraoptions" => $params['extraoptions'],
			"nfs-enabled" => array_boolval($params, 'nfs-enabled'),
			"smb-enabled" => array_boolval($params, 'smb-enabled'),
			"ssh-enabled" => array_boolval($params, 'ssh-enabled'),
			"pam-enabled" => array_boolval($params, 'pam-enabled')
		);

		// Set the configuration object
		$xpath = "//services/kerberos";
		if(FALSE === $xmlConfig -> replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}

		// Notify of configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher -> notify(OMV_NOTIFY_MODIFY,"org.openmediavault.services.kerberos", $object);

		return $object;
	}

	/**
	 * Get the current keytab.
	 *
	 * @param params An array containing all of the parameters to this function.
	 * @param context The context of the caller.
	 * @return A list of keytab items.
	 */
	function getKeyTab($params, $context) {
		// Validate the RPC caller context
		$this -> validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));

		$cmd = "export LANG=C; echo \"rkt /etc/krb5.keytab\nlist\" | ktutil 2>&1";
		@OMVUtil::exec($cmd, $output, $result);

		if($result !== 0) {
			return FALSE;
		}

		$list = array();

		foreach($output as $outputv) {
			$outputv = preg_split("/\s+/", trim($outputv));

			// We need to throw away all of the lines that don't start with a number
			if(is_numeric($outputv[0])) {
				$entry = array(
					"slot" => intval($outputv[0]),
					"version" => intval($outputv[1]),
					"principal" => $outputv[2]
				);

				if(preg_match($this -> principalRegex, $principal, $matches)) {
					$entry["type"] = $matches[1];
					$entry["object"] = $matches[2];
					$entry["realm"] = $matches[3];
				}

				$list[] = $entry;
			}
		}
		
		return $list;
	}

	/**
	 * Load a key from a KDC into the local keytab.
	 *
	 * Required parameters are an admin principal that has rights
	 * to read the desired keytab, the admin principal's password,
	 * and the target keytab to load.
	 *
	 * @param params An array containing all of the parameters to this function.
	 * @param context The context of the caller.
	 * @return The output from kadmin.
	 */
	function addKeyFromKeyServer($params, $context) {
		// Validate the RPC caller context
		$this -> validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));

		// Validate the parameters of the RPC servicemethod.
		$this -> validateMethodParams($params, '{
			"type": "object",
			"properties": {
				"adminPassword": {"type": "string"},
				"adminPrincipal": {"type": "string"},
				"targetPrincipal": {"type": "string"}
			}
		}');

		// Since kadmin's status reporting is rediculous, we're going
		// to see if this worked by reading the keytab before and after,
		// and comparing.
		$currentKeytab = $this -> getKeyTab($params, $context);

		$cmd = sprintf("export LANG=C; kadmin -p \"%s\" -w \"%s\" -q \"ktadd \\\"%s\\\"\" 2>&1",
				$params['adminPrincipal'],
				$params['adminPassword'],
				$params['targetPrincipal']
		);
		$result = $this -> exec($cmd, $output);

		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				sprintf("%s: %s", gettext("Failed to add desired key from KDC"), implode("\n", $output)));
		}

		$newKeyTab = $this -> getKeyTab($params, $context);

		// If the number of keys in the old and new keytabs is identical,
		// we failed.
		if(count($currentKeyTab) === count($newKeyTab)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				sprintf("%s: %s", gettext("Failed to add desired key from KDC"), implode("\n", $output)));
		}

		return $output;
	}

	/**
	 * Remove one or more keys from the local keytab.
	 *
	 * This simply takes a list of slot numbers.
	 *
	 * @param params An array containing all of the parameters to this function.
	 * @param context The context of the caller.
	 * @return The output from ktutil.
	 */
	function removeKey($params, $context) {
		// Validate the RPC caller context
		$this -> validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));

		// Validate the parameters of the RPC servicemethod.
		$this -> validateMethodParams($params, '{
			"type": "object",
			"properties": {
				"slots": {
					"type": "array",
					"minItems": 1,
					"items": {
						"type": "integer"
					},
					"uniqueItems": true
				}
			}
		}');

		// ktutil is also rediculous. The slot number is monotonically
		// increasing, with no gaps. As soon as we remove a key, all keys
		// move down a slot. This means we can't actually use the slot number
		// as given to remove keys. We need to compute the correct slot number
		// to use based on the changes we're making. It's so much fun.
		//
		// However, if this operating will result in no keys, ktutil will simply
		// write no file. So, if there is only one key in the keytab or we're removing
		// the same number as in the keytab, simply delete the keytab file.
		$currentKeyTab = $this -> getKeyTab($params, $context);

		if(count($params['slots']) === 1 || count($params['slots']) === count($currentKeyTab)) {
			$cmd = "export LANG=C; rm -f /etc/krb5.keytab";
		} else {
			arsort($params['slots']);

			$cmd = "export LANG=C; echo \"rkt /etc/krb5.keytab\n";

			// So, lets do this N times, one for each slot to be removed.
			// As we do this, every time we remove one, we decrement the rest
			// of the slots by 1.
			for($slot = 0; $slot < count($params['slots']); ++$slot) {
				$cmd .= "delent " . $params['slots'][$slot] . "\n";

				for($num = $slot+1; $num < count($params['slots']); ++$num) {
					$params['slots'][$num]--;
				}
			}

			$cmd .= "wkt /tmp/krb5.keytab\" | ktutil && mv /tmp/krb5.keytab /etc/krb5.keytab 2>&1";
		}

		$result = $this -> exec($cmd, $output);

		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				sprintf("%s: %s", gettext("Failed to remove key(s) from key tab"), implode("\n", $output)));
		}

		return $output;
	}

	/**
	 * Create a new key on the KDC and load it into the local keytab.
	 *
	 * Required parameters are an admin principal that has rights
	 * to read the desired keytab, the admin principal's password,
	 * the new principal to create, and either a password for the key
	 * or instructions to randomly generate a key.
	 *
	 * @param params An array containing all of the parameters to this function.
	 * @param context The context of the caller.
	 * @return The output from kadmin.
	 */
	function createKey($params, $context) {
		// Validate the RPC caller context
		$this -> validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));

		// Validate the parameters of the RPC servicemethod.
		$this -> validateMethodParams($params, '{
			"type": "object",
			"properties": {
				"adminPassword": {"type": "string"},
				"adminPrincipal": {"type": "string"},
				"newPrincipal": {"type": "string"},
				"newPrincipalKey": {"type": "string", "optional": true},
				"randomKey": {"type": "boolean"}
			}
		}');

		// First, create the key on the key server. Order of the query matters,
		// so this is a bit weird.
		$cmd = sprintf("export LANG=C; kadmin -p \"%s\" -w \"%s\" -q \"addprinc",
				$params['adminPrincipal'],
				$params['adminPassword']
		);

		if($params['randomKey']) {
			$cmd .= " -randkey";
		} else {
			$cmd .= " -pw \\\"" . $params['newPrincipalKey'] . "\\\"";
		}

		$cmd .= " \\\"" . $params['newPrincipal'] . "\\\"\"";

		$result = $this -> exec($cmd, $output);

		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				sprintf("%s: %s", gettext("Failed to create new key on KDC"), implode("\n", $output)));
		}

		// Now, use addKeyFromKeyServer to add the key to the keytab.
		$addParams = array(
			"adminPassword" => $params['adminPassword'],
			"adminPrincipal" => $params['adminPrincipal'],
			"targetPrincipal" => $params['newPrincipal']
		);

		$output .= $this -> addKeyFromKeyServer($addParams, $context);

		return $output;
	}
}

$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr -> registerService(new OMVRpcServiceKerberos());