<?php

/**
 * Copyright (C) 2009-2013 Volker Theile <volker.theile@openmediavault.org>
 * Copyright (C) 2013-2014 OpenMediaVault Plugin Developers
 * Copyright (C) 2015	   Ilya Kogan <ikogan@flarecode.com>
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
		$object['enable'] = boolvalEx($object['enable']);
		$object['logging'] = boolvalEx($object['logging']);
		$object['nfs-enabled'] = boolvalEx($object['nfs-enabled']);
		$object['smb-enabled'] = boolvalEx($object['smb-enabled']);
		$object['ssh-enabled'] = boolvalEx($object['ssh-enabled']);
		$object['pam-enabled'] = boolvalEx($object['pam-enabled']);

		return $object;
	}

	/**
	 * Set the LDAP configuration settings.
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
				"kdc": {"type": "string"},
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
		$object -> array(
			"enable" => array_boolval($params, 'enable'),
			"realm" => trim($params['realm']),
			"kdc" => trim($params['kdc']),
			"adminServer" => trim($params['adminServer']),
			"logging" => array_boolval($params, 'adminServer'),
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
}

$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr -> registerService(new OMVRpcServiceKerberos());