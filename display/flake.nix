{
  description = "Windstille Display Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "gitlab:grumbel/cmake-modules";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    babyxml.url = "gitlab:grumbel/babyxml";
    babyxml.inputs.nixpkgs.follows = "nixpkgs";
    babyxml.inputs.flake-utils.follows = "flake-utils";
    babyxml.inputs.tinycmmc.follows = "tinycmmc";

    logmich.url = "gitlab:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.flake-utils.follows = "flake-utils";
    logmich.inputs.tinycmmc.follows = "tinycmmc";

    geomcpp.url = "gitlab:grumbel/geomcpp";
    geomcpp.inputs.nixpkgs.follows = "nixpkgs";
    geomcpp.inputs.flake-utils.follows = "flake-utils";
    geomcpp.inputs.tinycmmc.follows = "tinycmmc";

    surfcpp.url = "gitlab:grumbel/surfcpp";
    surfcpp.inputs.nixpkgs.follows = "nixpkgs";
    surfcpp.inputs.flake-utils.follows = "flake-utils";
    surfcpp.inputs.tinycmmc.follows = "tinycmmc";
    surfcpp.inputs.geomcpp.follows = "geomcpp";
    surfcpp.inputs.logmich.follows = "logmich";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc, babyxml, geomcpp, logmich, surfcpp }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          wstdisplay = pkgs.stdenv.mkDerivation {
            pname = "wstdisplay";
            version = "0.3.0";
            src = nixpkgs.lib.cleanSource ./.;
            cmakeFlags = [ "-DBUILD_EXTRA=ON" ];
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.gcc
              pkgs.pkgconfig
            ];
            buildInputs = [
              tinycmmc.defaultPackage.${system}

              pkgs.gtest
            ];
            propagatedBuildInputs = [
              babyxml.defaultPackage.${system}
              geomcpp.defaultPackage.${system}
              logmich.defaultPackage.${system}
              surfcpp.defaultPackage.${system}

              pkgs.freetype
              pkgs.libGL
              pkgs.libGLU
              pkgs.glew
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
