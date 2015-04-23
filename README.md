# Kerberos plugin for OpenMediaVault

This plugin implements a simple UI for configuring MIT Kerberos
for OpenMediaVault and integrating it with NFS and Samba.

Note that this is not intended to be a transparent plugin that integrates
Kerberos with the existing OpenMediaVault user/group/directory framework.
Rather, this provides an interface for the `/etc/krb5.conf` and
`/etc/krb5.keytab` files.

Additionally, this only provides support for Kerberos *clients*. There is
no support for configuring and maintaining a KDC (although it does
have the capability to add keys to a KDC).

It supports the following:

- Configuring the local Kerberos client.
- Enabling Kerberos in NFS, Samba, SSH, and PAM.
- Listing the contents of the local keytab.
- Removing individual keys from the keytab.
- Loading keys from the configured KDC into the local keytab.
- Creating a new key on the KDC and loading it into the local keytab.

There are a few important caveats:

1. In terms of NFS, it only enables support for it globally, the
   `sec=krb5`, `sec=krb5p`, or `sec=krb5i` options still need to be
   added to each share manually in the NFS configuration UI.
2. The locations of the Kerberos config files and keytab are hardcoded.
3. Automatic SSH support only works on OpenMediaVault 2.0.3 or later as
   it requires my SSH `mkconf` script patch. For older versions, simply
   add the following as extra options in the SSH configuration:

   ```
   KerberosAuthentication yes
   GSSAPIAuthentication yes
   ```
4. Creating a new key assumes you also want to load it to the local keytab,
   there is no way to create a key on the KDC and *not* load it except to
   create it first, then remove it. That seems a bit silly and isn't the
   intent of this plugin.

If you're using openmediavault-ldap, it may be useful to use
openmediavault-ldap 2.0 or greater in case you want to disable LDAP's PAM
integration in leu of Kerberos. In this way, you can use LDAP's NSS support
while taking advantage of the Kerberos ticket that `libpam-krb5` retrieves.

This has been tested on OpenMediaVault 1.17 and not 2.0 quite yet.