# oOOmod
Update open/libreoffice configuration from an XML file

## Features
oOOmod is a tool to update all the 'registrymodifications.xcu' files found in directories 'libreoffice/4' of the current user profil path.

The Windows version automatically tries to set the firstname, the surname and the user's initials by querying the domain server. The linux version does not implement this feature.


## Usage

```
  oOOmod config.xcu
```
where the configuration file 'config.xcu' is an XML file containing the changes to be applied and could be for example:
  ```
<?xml version="1.0" encoding="UTF-8" ?>
<oor:items xmlns:oor="http://openoffice.org/2001/registry" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
<item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="o" oor:op="fuse"><value>My organisation</value></prop></item>
<item oor:path="/org.openoffice.Setup/L10N"><prop oor:name="ooLocale" oor:op="fuse"><value>fr</value></prop></item>
<item oor:path="/org.openoffice.Setup/L10N"><prop oor:name="ooSetupSystemLocale" oor:op="fuse"><value>fr</value></prop></item>
<item oor:path="/org.openoffice.Office.Common/Save/Document"><prop oor:name="LoadPrinter" oor:op="fuse"><value>false</value></prop></item>
</oor:items>
```
Please note that the name 'config.xcu' is not fixed. It could be 'config.xml' or 'myconf.xml'.

If 'soffice.bin' is running then the process is not executed.

## License
GPLv3 license

Copyleft 2021 - Noël Martinon
