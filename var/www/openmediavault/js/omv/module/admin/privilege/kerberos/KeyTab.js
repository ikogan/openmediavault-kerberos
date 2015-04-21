/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @author    Ilya Kogan <ikogan@flarecode.com>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
 * @copyright Copyright (c)      2015 Ilya Kogan
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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/panel/Textarea.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/Tab.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

 /**
 * @class OMV.module.admin.privilege.kerberos.KeyTab
 * @derived OMV.workspace.window.Form
 *
 * List of keys in the keytab file.
 */
Ext.define("OMV.module.admin.privilege.kerberos.KeyTab", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.privilege.kerberos.AddKey"
	],

	hideEditButton: true,

	stateful: true,
	stateId: "17a383a3-f68a-45ef-969e-bbe75f845396",
	columns: [{
		text: _("Slot"),
		sortable: true,
		dataIndex: "slot",
		stateId: "slot"
	},{
		text: _("Version"),
		sortable: true,
		dataIndex: "version",
		stateId: "version"
	},{
		text: _("Principal"),
		sortable: true,
		dataIndex: "principal",
		stateId: "principal"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: 'slot',
					fields: [
						{ name: 'slot', type: 'number' },
						{ name: 'version', type: 'number' },
						{ name: 'principal', type: 'string' },
						{ name: 'type', type: 'string' },
						{ name: 'object', type: 'string' },
						{ name: 'realm', type: 'string' }
					]
				}),
				proxy: {
					type: 'rpc',
					rpcData: {
						service: 'Kerberos',
						method: 'getKeyTab'
					}
				}
			})
		});

		me.callParent(arguments);
	},

	onAddButton: function() {
		Ext.create("OMV.module.admin.privilege.kerberos.AddKey").show();
	},

	onDeleteButton: function() {
		var me = this;
		var records = me.getSelected();

		Ext.create("OMV.module.admin.privilege.kerberos.DeleteKeys", {
			keys: records,
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show()
	}
});

 /**
 * @class OMV.module.admin.privilege.kerberos.KeyTab
 * @derived OMV.workspace.window.Form
 *
 * Load a new key from the key server
 */
 Ext.define("OMV.module.admin.privilege.kerberos.AddKey", {
 	extend: "OMV.workspace.window.Form",

 	title: _("Add Key from Key Server"),
 	hideTopToolbar: true,
 	rpcService: "Kerberos",
 	rpcSetMethod: "addKeyFromKeyServer",

 	getFormItems: function() {
 		var me = this;

 		return [{
 			xtype: "textfield",
 			name: "adminPrincipal",
 			fieldLabel: _("Administration Principal"),
 			value: me.adminPrincipal
 		}, {
 			xtype: "passwordfield",
 			name: "adminPassword",
 			fieldLabel: _("Administration Password"),
 			value: me.adminPassword
 		}, {
 			xtype: "textfield",
 			name: "targetPrincipal",
 			fieldLabel: _("Target Principal"),
 			value: me.targetPrincipal
 		}]
 	}
 });

 OMV.WorkspaceManager.registerPanel({
 	id: "keytab",
 	path: "/privilege/kerberos",
 	text: _("Key Tab"),
 	position: 20,
 	className: "OMV.module.admin.privilege.kerberos.KeyTab"
 });