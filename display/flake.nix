{
  description = "Windstille Display Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";

    babyxml.url = "github:grumbel/babyxml";
    babyxml.inputs.nixpkgs.follows = "nixpkgs";
    babyxml.inputs.tinycmmc.follows = "tinycmmc";

    logmich.url = "github:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.tinycmmc.follows = "tinycmmc";

    geomcpp.url = "github:grumbel/geomcpp";
    geomcpp.inputs.nixpkgs.follows = "nixpkgs";
    geomcpp.inputs.tinycmmc.follows = "tinycmmc";

    SDL2-win32.url = "github:grumnix/SDL2-win32";
    SDL2-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL2-win32.inputs.tinycmmc.follows = "tinycmmc";

    glew-win32.url = "github:grumnix/glew-win32";
    glew-win32.inputs.nixpkgs.follows = "nixpkgs";
    glew-win32.inputs.tinycmmc.follows = "tinycmmc";

    freetype-win32.url = "github:grumnix/freetype-win32";
    freetype-win32.inputs.nixpkgs.follows = "nixpkgs";
    freetype-win32.inputs.tinycmmc.follows = "tinycmmc";

    surfcpp.url = "github:grumbel/surfcpp";
    surfcpp.inputs.nixpkgs.follows = "nixpkgs";
    surfcpp.inputs.tinycmmc.follows = "tinycmmc";
    surfcpp.inputs.geomcpp.follows = "geomcpp";
    surfcpp.inputs.logmich.follows = "logmich";
    surfcpp.inputs.SDL2-win32.follows = "SDL2-win32";
  };

  outputs = { self, nixpkgs, tinycmmc, babyxml, geomcpp, logmich, surfcpp, freetype-win32, SDL2-win32, glew-win32 }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      rec {
        packages = rec {
          default = wstdisplay;

          wstdisplay = pkgs.gcc12Stdenv.mkDerivation {
            pname = "wstdisplay";
            version = "0.3.0";

            src = nixpkgs.lib.cleanSource ./.;

            cmakeFlags = [ "-DBUILD_EXTRA=ON" ];

            postFixup =
              (nixpkgs.lib.optionalString pkgs.targetPlatform.isWindows ''
                mkdir -p $out/bin/
                find ${pkgs.windows.mcfgthreads} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                find ${pkgs.stdenv.cc.cc} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                ln -sfv ${SDL2-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.fmt_8}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.gtest}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.libsigcxx}/bin/*.dll $out/bin/
                ln -sfv ${freetype-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                ln -sfv ${glew-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/

                # FIXME: These should be handled in surfcpp or statically linked
                ln -sfv ${pkgs.libjpeg_original.overrideAttrs (oldAttrs: { meta = {}; })}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.libpng.overrideAttrs (oldAttrs: { meta = {}; })}/bin/*.dll $out/bin/
                ln -sfv ${pkgs.zlib.overrideAttrs (oldAttrs: { meta = {}; })}/bin/*.dll $out/bin/
            '');

            nativeBuildInputs = [
              pkgs.buildPackages.cmake
              pkgs.buildPackages.pkgconfig
            ];

            buildInputs = [
              tinycmmc.packages.${pkgs.system}.default

              pkgs.gtest
            ];

            propagatedBuildInputs = [
              babyxml.packages.${pkgs.system}.default
              geomcpp.packages.${pkgs.system}.default
              logmich.packages.${pkgs.system}.default
              surfcpp.packages.${pkgs.system}.default

              pkgs.libsigcxx
              pkgs.fmt_8

              (if pkgs.targetPlatform.isWindows
               then freetype-win32.packages.${pkgs.system}.default
               else pkgs.freetype)

              (if pkgs.targetPlatform.isWindows
               then glew-win32.packages.${pkgs.system}.default
               else pkgs.glew)

              (if pkgs.targetPlatform.isWindows
               then SDL2-win32.packages.${pkgs.system}.default
               else pkgs.SDL2)
            ];
          };
        };

        apps = rec {
          default = wstdisplay-geometry;

          wstdisplay-geometry = tinycmmc.inputs.flake-utils.lib.mkApp {
            drv = packages.wstdisplay;
            exePath = "/bin/wstdisplay-geometry";
          };
        };
      }
    );
}
