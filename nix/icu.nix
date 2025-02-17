{ version, sha256, pkgs, patches ? [], patchFlags ? [] }:

let
  lib = pkgs.lib;
  stdenv = pkgs.stdenv;
  fetchurl = pkgs.fetchurl;
  buildRootOnly = false;
  pname = "icu4c";

  baseAttrs = {
    src = fetchurl {
      url = "https://github.com/unicode-org/icu/releases/download/release-${lib.replaceChars [ "." ] [ "-" ] version}/icu4c-${lib.replaceChars [ "." ] [ "_" ] version}-src.tgz";
      inherit sha256;
    };

    postUnpack = ''
      sourceRoot=''${sourceRoot}/source
      echo Source root reset to ''${sourceRoot}
    '';

    # https://sourceware.org/glibc/wiki/Release/2.26#Removal_of_.27xlocale.h.27
    postPatch = if (stdenv.hostPlatform.libc == "glibc" || stdenv.hostPlatform.libc == "musl") && lib.versionOlder version "62.1"
      then "substituteInPlace i18n/digitlst.cpp --replace '<xlocale.h>' '<locale.h>'"
      else null; # won't find locale_t on darwin

    inherit patchFlags patches;

    preConfigure = ''
      sed -i -e "s|/bin/sh|${stdenv.shell}|" configure
      # $(includedir) is different from $(prefix)/include due to multiple outputs
      sed -i -e 's|^\(CPPFLAGS = .*\) -I\$(prefix)/include|\1 -I$(includedir)|' config/Makefile.inc.in
    '' + lib.optionalString stdenv.isAarch32 ''
      # From https://archlinuxarm.org/packages/armv7h/icu/files/icudata-stdlibs.patch
      sed -e 's/LDFLAGSICUDT=-nodefaultlibs -nostdlib/LDFLAGSICUDT=/' -i config/mh-linux
    '';

    configureFlags = [ "--disable-debug" ]
      ++ lib.optional (stdenv.isFreeBSD || stdenv.isDarwin) "--enable-rpath";

    enableParallelBuilding = true;

    meta = with lib; {
      description = "Unicode and globalization support library";
      homepage = "http://site.icu-project.org/";
      maintainers = with maintainers; [ raskin ];
      platforms = platforms.all;
    };
  };

  realAttrs = baseAttrs // {
    name = pname + "-" + version;

    outputs = [ "out" "dev" ];
    outputBin = "dev";

    # remove dependency on bootstrap-tools in early stdenv build
    postInstall = lib.optionalString stdenv.isDarwin ''
      sed -i 's/INSTALL_CMD=.*install/INSTALL_CMD=install/' $out/lib/icu/${version}/pkgdata.inc
    '' + (let
      replacements = [
        { from = "\${prefix}/include"; to = "${placeholder "dev"}/include"; } # --cppflags-searchpath
        { from = "\${pkglibdir}/Makefile.inc"; to = "${placeholder "dev"}/lib/icu/Makefile.inc"; } # --incfile
        { from = "\${pkglibdir}/pkgdata.inc"; to = "${placeholder "dev"}/lib/icu/pkgdata.inc"; } # --incpkgdatafile
      ];
    in ''
      substituteInPlace "$dev/bin/icu-config" \
        ${lib.concatMapStringsSep " " (r: "--replace '${r.from}' '${r.to}'") replacements}
    '');

    postFixup = ''moveToOutput lib/icu "$dev" '';
  };

  buildRootOnlyAttrs = baseAttrs // {
    name = pname + "-build-root-" + version;

    preConfigure = baseAttrs.preConfigure + ''
      mkdir build
      cd build
      configureScript=../configure
    '';

    postBuild = ''
      cd ..
      mv build $out
      echo "Doing build-root only, exiting now" >&2
      exit 0
    '';
  };

  attrs = if buildRootOnly
            then buildRootOnlyAttrs
          else realAttrs;
in
stdenv.mkDerivation attrs