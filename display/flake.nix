{
  description = "Windstille Display Engine";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    tinycmmc.url = "gitlab:grumbel/cmake-modules";
    # babyxml.url = "gitlab:grumbel/babyxml";
    # geomcpp.url = "gitlab:grumbel/geomcpp";
    # logmich.url = "gitlab:grumbel/logmich";
    # surfcpp.url = "gitlab:grumbel/surfcpp";
    babyxml.url = "git+file:///home/ingo/projects/babyxml/trunk/babyxml";
    geomcpp.url = "git+file:///home/ingo/projects/geomcpp/trunk";
    logmich.url = "git+file:///home/ingo/projects/logmich/trunk";
    surfcpp.url = "git+file:///home/ingo/projects/surfcpp/trunk";
  };

  outputs = { self, nix, nixpkgs, flake-utils, tinycmmc, babyxml, geomcpp, logmich, surfcpp }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          wstdisplay = pkgs.stdenv.mkDerivation {
            pname = "wstdisplay";
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
              babyxml.defaultPackage.${system}
              geomcpp.defaultPackage.${system}
              logmich.defaultPackage.${system}
              surfcpp.defaultPackage.${system}
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
            ];
           };
        };
        defaultPackage = packages.wstdisplay;
      });
}
