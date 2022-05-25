rec {
  description = "Windstille GUI Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "gitlab:grumbel/cmake-modules";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    logmich.url = "gitlab:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.flake-utils.follows = "flake-utils";
    logmich.inputs.tinycmmc.follows = "tinycmmc";

    geomcpp.url = "gitlab:grumbel/geomcpp";
    geomcpp.inputs.nixpkgs.follows = "nixpkgs";
    geomcpp.inputs.flake-utils.follows = "flake-utils";
    geomcpp.inputs.tinycmmc.follows = "tinycmmc";

    priocpp.url = "gitlab:grumbel/priocpp";
    priocpp.inputs.nixpkgs.follows = "nixpkgs";
    priocpp.inputs.flake-utils.follows = "flake-utils";
    priocpp.inputs.tinycmmc.follows = "tinycmmc";
    priocpp.inputs.logmich.follows = "logmich";
    priocpp.inputs.sexpcpp.follows = "sexpcpp";

    surfcpp.url = "gitlab:grumbel/surfcpp";
    surfcpp.inputs.nixpkgs.follows = "nixpkgs";
    surfcpp.inputs.flake-utils.follows = "flake-utils";
    surfcpp.inputs.tinycmmc.follows = "tinycmmc";
    surfcpp.inputs.geomcpp.follows = "geomcpp";
    surfcpp.inputs.logmich.follows = "logmich";

    babyxml.url = "gitlab:grumbel/babyxml";
    babyxml.inputs.nixpkgs.follows = "nixpkgs";
    babyxml.inputs.flake-utils.follows = "flake-utils";
    babyxml.inputs.tinycmmc.follows = "tinycmmc";

    sexpcpp.url = "gitlab:lispparser/sexp-cpp";
    sexpcpp.inputs.nixpkgs.follows = "nixpkgs";
    sexpcpp.inputs.flake-utils.follows = "flake-utils";
    sexpcpp.inputs.tinycmmc.follows = "tinycmmc";

    wstinput.url = "gitlab:windstille/wstinput";
    wstinput.inputs.nixpkgs.follows = "nixpkgs";
    wstinput.inputs.flake-utils.follows = "flake-utils";
    wstinput.inputs.tinycmmc.follows = "tinycmmc";
    wstinput.inputs.logmich.follows = "logmich";
    wstinput.inputs.priocpp.follows = "priocpp";
    wstinput.inputs.sexpcpp.follows = "sexpcpp";

    wstdisplay.url = "gitlab:windstille/wstdisplay";
    wstdisplay.inputs.nixpkgs.follows = "nixpkgs";
    wstdisplay.inputs.flake-utils.follows = "flake-utils";
    wstdisplay.inputs.tinycmmc.follows = "tinycmmc";
    wstdisplay.inputs.geomcpp.follows = "geomcpp";
    wstdisplay.inputs.babyxml.follows = "babyxml";
    wstdisplay.inputs.surfcpp.follows = "surfcpp";
    wstdisplay.inputs.logmich.follows = "logmich";

    wstsound.url = "gitlab:windstille/wstsound";
    wstsound.inputs.nixpkgs.follows = "nixpkgs";
    wstsound.inputs.flake-utils.follows = "flake-utils";
    wstsound.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils,
              tinycmmc, logmich, geomcpp, priocpp, surfcpp, babyxml, sexpcpp,
              wstinput, wstdisplay, wstsound }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          wstgui = pkgs.stdenv.mkDerivation {
            pname = "wstgui";
            version = "0.3.0";
            src = nixpkgs.lib.cleanSource ./.;
            cmakeFlags = [ "-DBUILD_EXTRA=ON" ];
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkgconfig
              pkgs.makeWrapper
            ];
            postFixup = ''
                wrapProgram $out/bin/wstgui \
                  --prefix LIBGL_DRIVERS_PATH ":" "${pkgs.mesa.drivers}/lib/dri" \
                  --prefix LD_LIBRARY_PATH ":" "${pkgs.mesa.drivers}/lib"
            '';
            buildInputs = [
              babyxml.defaultPackage.${system}
            ];
            propagatedBuildInputs = [
              geomcpp.defaultPackage.${system}
              logmich.defaultPackage.${system}
              priocpp.defaultPackage.${system}
              sexpcpp.defaultPackage.${system}
              surfcpp.defaultPackage.${system}
              tinycmmc.defaultPackage.${system}
              wstdisplay.defaultPackage.${system}
              wstinput.defaultPackage.${system}
              wstsound.defaultPackage.${system}

              pkgs.freetype
              pkgs.libGL
              pkgs.libGLU
              pkgs.glew
              pkgs.gtest

              pkgs.fmt
              pkgs.glm
              pkgs.SDL2
              pkgs.libjpeg
              pkgs.libpng
              pkgs.libsigcxx
              pkgs.imagemagick6
              pkgs.libexif

              pkgs.openal
              pkgs.libvorbis
              pkgs.libogg
              pkgs.opusfile
              pkgs.mpg123
              pkgs.libmodplug
              pkgs.gtest

              pkgs.jsoncpp
            ];
           };
        };
        defaultPackage = packages.wstgui;
      });
}
