/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.privilege.kerberos.Settings", {
    extend : "OMV.workspace.form.Panel",

    rpcService   : "Kerberos",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    getFormItems : function () {
        return [{
            xtype         : "fieldset",
            title         : _("General settings"),
            fieldDefaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : false
            },{
                xtype      : "textfield",
                name       : "realm",
                fieldLabel : _("Realm"),
                allowBlank : false,
                vtype: "domainname",
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("FQDN in all caps. It should match a real domain that a DNS server (even if local) will resolve. Note that the actual domain does not have to be in all uppercase.")
                }]
            },{
                xtype      : "textfield",
                name       : "kdcs",
                fieldLabel : _("KDCs"),
                allowBlank : false,
                vtype: "domainnameIPList",
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("FQDN or IP address of the Kerberos Key Distribution Servers. Separate multiple servers with commas.")
                }]
            },{
                xtype      : "textfield",
                name       : "adminServer",
                fieldLabel : _("Admin Server"),
                allowBlank : false,
                vtype: "domainnameIP",
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("FQDN or IP address of the Kerberos Admin Server.")
                }]
            },{
                xtype      : "checkbox",
                name       : "logging",
                fieldLabel : _("Logging"),
                checked    : true
            },{
                xtype: "textarea",
                name: "extraoptions",
                fieldLabel: _("Extra options"),
                allowBlank: true,
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("Please check the <a href='http://linux.die.net/man/5/krb5.conf' target='_blank'>manual page</a> for more details.")
                }]
            }]
        },{
            xtype          : "fieldset",
            title          : _("Integrations"),
            fieldDefaults  : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "checkbox",
                name       : "nfs-enabled",
                fieldLabel : _("NFS"),
                checked    : true,
                boxLabel   : _("Enable support for Kerberos security for all NFS shares.")
            }, {
                xtype      : "checkbox",
                name       : "smb-enabled",
                fieldLabel : _("SMB/CIFS"),
                checked    : true,
                boxLabel   : _("Enable support for Kerberos authentication for SMB/CIFS.")
            }, {
                xtype      : "checkbox",
                name       : "ssh-enabled",
                fieldLabel : _("SSH"),
                checked    : true,
                boxLabel   : _("Enable support for Kerberos and GSSAPI authentication for SSH.")
            }, {
                xtype      : "checkbox",
                name       : "pam-enabled",
                fieldLabel : _("PAM"),
                checked    : true,
                boxLabel   : _("Enable support for system-wide Kerberos authentication via PAM.")
            }]
        }]
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/privilege/kerberos",
    text      : _("Settings"),
    position  : 15,
    className : "OMV.module.admin.privilege.kerberos.Settings"
});