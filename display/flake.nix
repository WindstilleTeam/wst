{
  description = "Windstille Display Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    babyxml.url = "git+https://github.com/grumbel/babyxml.git";
    babyxml.inputs.nixpkgs.follows = "nixpkgs";

    logmich.url = "git+https://github.com/logmich/logmich.git";
    logmich.inputs.nixpkgs.follows = "nixpkgs";

    geomcpp.url = "git+https://github.com/grumbel/geomcpp.git";
    geomcpp.inputs.nixpkgs.follows = "nixpkgs";

    SDL2-win32.url = "git+https://github.com/grumnix/SDL2-win32.git";
    SDL2-win32.inputs.nixpkgs.follows = "nixpkgs";

    freetype-win32.url = "git+https://github.com/grumnix/freetype-win32.git";
    freetype-win32.inputs.nixpkgs.follows = "nixpkgs";

    surfcpp.url = "git+https://github.com/grumbel/surfcpp.git";
    surfcpp.inputs.nixpkgs.follows = "nixpkgs";
    surfcpp.inputs.geomcpp.follows = "geomcpp";
    surfcpp.inputs.logmich.follows = "logmich";
    surfcpp.inputs.SDL2-win32.follows = "SDL2-win32";
  };

  outputs = { self, nixpkgs, flake-utils, babyxml, geomcpp, logmich, surfcpp, freetype-win32, SDL2-win32 }:
    let
      # Read the base from VERSION.
      versionBase = nixpkgs.lib.strings.removeSuffix "\n" (builtins.readFile ./VERSION);
      gitRev = "${self.shortRev or self.dirtyShortRev or "dirty"}";
      isDev = nixpkgs.lib.strings.hasInfix "-dev" versionBase;
      version =
        if isDev then
          "${versionBase}.${toString (self.revCount or 0)}+g${gitRev}"
        else
          versionBase;

      # Support Windows cross as tinycmmc did
      eachSystem = flake-utils.lib.eachSystem (flake-utils.lib.defaultSystems ++ [ "x86_64-windows" "i686-windows" ]);
      pkgsFromSystem = system:
        if system == "x86_64-windows" then nixpkgs.legacyPackages.x86_64-linux.pkgsCross.mingwW64
        else if system == "i686-windows" then nixpkgs.legacyPackages.x86_64-linux.pkgsCross.mingw32
        else nixpkgs.legacyPackages.${system};
    in
    eachSystem (system:
      let
        pkgs = pkgsFromSystem system;
      in
      rec {
        packages = rec {
          default = wstdisplay;

          wstdisplay = pkgs.stdenv.mkDerivation {
            pname = "wstdisplay";
            inherit version;

            src = nixpkgs.lib.cleanSource ./.;

            cmakeFlags = [
              "-DBUILD_EXTRA=ON"
              "-DPROJECT_VERSION_FULL=${version}"
            ];

            postFixup =
              (nixpkgs.lib.optionalString pkgs.stdenv.hostPlatform.isWindows ''
                mkdir -p $out/bin/
                find ${pkgs.windows.mcfgthreads} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                find ${pkgs.stdenv.cc.cc} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                ln -sfv ${SDL2-win32.packages.${pkgs.stdenv.hostPlatform.system}.default}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.gtest}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.libsigcxx_3_0}/bin/*.dll $out/bin/
                ln -sfv ${freetype-win32.packages.${pkgs.stdenv.hostPlatform.system}.default}/bin/*.dll $out/bin/

                # FIXME: These should be handled in surfcpp or statically linked
                ln -sfv ${pkgs.libjpeg_original.overrideAttrs (oldAttrs: { meta = {}; })}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.libpng.overrideAttrs (oldAttrs: { meta = {}; })}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.zlib.overrideAttrs (oldAttrs: { meta = {}; })}/bin/*.dll $out/bin/
            '');

            nativeBuildInputs = [
              pkgs.buildPackages.cmake
              pkgs.buildPackages.pkg-config
            ];

            buildInputs = [
              pkgs.gtest
            ];

            propagatedBuildInputs = [
              babyxml.packages.${pkgs.stdenv.hostPlatform.system}.default
              geomcpp.packages.${pkgs.stdenv.hostPlatform.system}.default
              logmich.packages.${pkgs.stdenv.hostPlatform.system}.default
              surfcpp.packages.${pkgs.stdenv.hostPlatform.system}.default

              pkgs.libsigcxx_3_0

              (if pkgs.stdenv.hostPlatform.isWindows
               then freetype-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
               else pkgs.freetype)

              (if pkgs.stdenv.hostPlatform.isWindows
               then SDL2-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
               else pkgs.SDL2)

              # OpenGL via libglvnd (no X11 pulled in by GLEW anymore; GLAD is vendored)
              pkgs.libGL
              pkgs.libGLU
            ];
          };
        };

        apps = rec {
          default = wstdisplay-geometry;

          wstdisplay-geometry = flake-utils.lib.mkApp {
            drv = packages.wstdisplay;
            exePath = "/bin/wstdisplay-geometry";
          };
        };
      }
    );
}
