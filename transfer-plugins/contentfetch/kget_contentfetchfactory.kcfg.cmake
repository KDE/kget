<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE kcfg SYSTEM "http://www.kde.org/standards/kcfg/1.0/kcfg.dtd">
<kcfg>
  <kcfgfile name="kget_contentfetchfactory.rc"/>
   <group name="UserScripts">
    <entry name="UrlRegexpList" type="StringList" key="RegexpItems">
      <label>List of the Regexp to match input Url</label>
      <default>youtube.*watch</default>
    </entry>
    <entry name="PathList" type="StringList" key="PathItems">
      <label>Urls list of the available search engines</label>
      <default>${DATA_INSTALL_DIR}/kget/content_fetch_scripts/youtubedl/kget_youtubedl.py</default>
    </entry>
    <entry name="DescriptionList" type="StringList" key="DescriptionItems">
      <label>List of descriptions for user scripts</label>
      <default>A youtube flash video downloader</default>
    </entry>
    <entry name="EnableList" type="IntList" key="EnableItems">
      <label>List of whether the script is enabled</label>
      <default>1</default>
    </entry>
  </group>
</kcfg>
