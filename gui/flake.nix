{
  description = "Windstille GUI Engine";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    tinycmmc.url = "gitlab:grumbel/cmake-modules";
    geomcpp.url = "git+file:///home/ingo/projects/geomcpp/trunk";
    wstinput.url = "git+file:///home/ingo/projects/windstille/trunk/wstinput";
    wstdisplay.url = "git+file:///home/ingo/projects/windstille/trunk/wstdisplay";
    wstsound.url = "git+file:///home/ingo/projects/windstille/trunk/wstsound";
  };

  outputs = { self, nix, nixpkgs, flake-utils, tinycmmc, geomcpp, wstinput, wstdisplay, wstsound }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          wstgui = pkgs.stdenv.mkDerivation {
            pname = "wstgui";
            version = "0.3.0";
            src = nixpkgs.lib.cleanSource ./.;
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.gcc
              pkgs.pkgconfig
              tinycmmc.defaultPackage.${system}
            ];
            buildInputs = [
              geomcpp.defaultPackage.${system}
              wstinput.defaultPackage.${system}
              wstsound.defaultPackage.${system}
              wstdisplay.defaultPackage.${system}

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
            ];
           };
        };
        defaultPackage = packages.wstgui;
      });
}
