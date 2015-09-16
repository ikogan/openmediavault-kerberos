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
	selType: "checkboxmodel",
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
		stateId: "principal",
		flex: 1
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

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);

		Ext.Array.push(items, [{
			id: me.getId() + "-create",
			xtype: "button",
			text: _("Create Principal"),
			icon: "images/upload.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onCreateButton, me, [me]),
			scope: me,
			disabled: false
		}, {
			id: me.getId() + "-reload",
			xtype: "button",
			text: _("Reload"),
			icon: "images/refresh.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onReloadButton, me, [me]),
			scope: me,
			disabled: false
		}]);

		return items;
	},

	onReloadButton: function() {
		this.doReload();
	},

	onCreateButton: function() {
		var me = this;

		Ext.create("OMV.module.admin.privilege.kerberos.CreateKey", {
			listeners: {
				scope: me,
				submit: function() {
					me.doReload();
				}
			}
		}).show();		
	},

	onAddButton: function() {
		var me = this;

		Ext.create("OMV.module.admin.privilege.kerberos.AddKey", {
			listeners: {
				scope: me,
				submit: function() {
					me.doReload();
				}
			}
		}).show();
	},

	onDeleteButton: function() {
		var me = this;
		var records = me.getSelection();

		Ext.MessageBox.confirm(_('Delete Keys?'), _('Are you sure you want to remove these keys?'), function(button) {
			if(button === 'yes') {
				var slots = [];
				Ext.each(records, function(record) {
					slots.push(record.raw.slot);
				});

				OMV.Rpc.request({
					scope: me,
					callback: function(id, success, response) {
						if(success) {
							me.doReload();
						} else {
           					Ext.Msg.alert(_('Error'), response.message);
						}
					},
					rpcData: {
						service: "Kerberos",
						method: "removeKey",
						params: {
							'slots': slots
						}
					}
				});
			}
		}, me);
	}
});

/**
 * @class OMV.module.admin.privilege.kerberos.AddKey
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
			value: me.adminPrincipal,
			allowBlank: false
		}, {
			xtype: "passwordfield",
			name: "adminPassword",
			fieldLabel: _("Administration Password"),
			value: me.adminPassword,
			allowBlank: false
		}, {
			xtype: "textfield",
			name: "targetPrincipal",
			fieldLabel: _("Target Principal"),
			value: me.targetPrincipal,
			allowBlank: false
		}]
	}
});

/**
 * @class OMV.module.admin.privilege.kerberos.CreateKey
 * @derived OMV.workspace.window.Form
 *
 * Create a new key on the key server and load it into the
 * local keytab.
 */
Ext.define("OMV.module.admin.privilege.kerberos.CreateKey", {
	extend: "OMV.workspace.window.Form",

	title: _("Create Principal"),
	hideTopToolbar: true,
	rpcService: "Kerberos",
	rpcSetMethod: "createKey",

	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: "newPrincipalKey",
			conditions: [
				{ name: "randomKey", value: true }
			],
			properties: ["disabled", "allowBlank"]
		}]
	}],

	getFormItems: function() {
		var me = this;

		return [{
			xtype: "textfield",
			name: "adminPrincipal",
			fieldLabel: _("Administration Principal"),
			value: me.adminPrincipal,
			allowBlank: false
		}, {
			xtype: "passwordfield",
			name: "adminPassword",
			fieldLabel: _("Administration Password"),
			value: me.adminPassword,
			allowBlank: false
		}, {
			xtype: "textfield",
			name: "newPrincipal",
			fieldLabel: _("New Principal"),
			value: me.newPrincipal,
			allowBlank: false
		}, {
			xtype: "passwordfield",
			name: "newPrincipalKey",
			fieldLabel: _("New Principal Key"),
			value: me.newPrincipalKey
		}, {
			xtype: "checkbox",
			name: "randomKey",
			fieldLabel: _("Use Random Key"),
			checked: true
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