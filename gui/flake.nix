{
  description = "Windstille GUI Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    logmich.url = "git+https://github.com/logmich/logmich.git";
    logmich.inputs.nixpkgs.follows = "nixpkgs";

    geomcpp.url = "git+https://github.com/grumbel/geomcpp.git";
    geomcpp.inputs.nixpkgs.follows = "nixpkgs";

    priocpp.url = "git+https://github.com/grumbel/priocpp.git";
    priocpp.inputs.nixpkgs.follows = "nixpkgs";
    priocpp.inputs.logmich.follows = "logmich";

    surfcpp.url = "git+https://github.com/grumbel/surfcpp.git";
    surfcpp.inputs.nixpkgs.follows = "nixpkgs";
    surfcpp.inputs.geomcpp.follows = "geomcpp";
    surfcpp.inputs.logmich.follows = "logmich";

    babyxml.url = "git+https://github.com/grumbel/babyxml.git";
    babyxml.inputs.nixpkgs.follows = "nixpkgs";

    sexpcpp.url = "git+https://github.com/lispparser/sexp-cpp.git";
    sexpcpp.inputs.nixpkgs.follows = "nixpkgs";

    wstinput.url = "git+https://github.com/windstilleteam/wstinput.git";
    wstinput.inputs.nixpkgs.follows = "nixpkgs";
    wstinput.inputs.logmich.follows = "logmich";
    wstinput.inputs.priocpp.follows = "priocpp";
    wstinput.inputs.sexpcpp.follows = "sexpcpp";

    wstdisplay.url = "git+https://github.com/windstilleteam/wstdisplay.git";
    wstdisplay.inputs.nixpkgs.follows = "nixpkgs";
    wstdisplay.inputs.geomcpp.follows = "geomcpp";
    wstdisplay.inputs.babyxml.follows = "babyxml";
    wstdisplay.inputs.surfcpp.follows = "surfcpp";
    wstdisplay.inputs.logmich.follows = "logmich";

    wstsound.url = "git+https://github.com/windstilleteam/wstsound.git";
    wstsound.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, flake-utils,
              logmich, geomcpp, priocpp, surfcpp, babyxml, sexpcpp,
              wstinput, wstdisplay, wstsound }:
    let
      versionBase = nixpkgs.lib.strings.removeSuffix "\n" (builtins.readFile ./VERSION);
      gitRev = "${self.shortRev or self.dirtyShortRev or "dirty"}";
      isDev = nixpkgs.lib.strings.hasInfix "-dev" versionBase;
      version =
        if isDev then
          "${versionBase}.${toString (self.revCount or 0)}+g${gitRev}"
        else
          versionBase;

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
      {
        packages = rec {
          default = wstgui;

          wstgui = pkgs.stdenv.mkDerivation {
            pname = "wstgui";
            inherit version;

            src = nixpkgs.lib.cleanSource ./.;

            cmakeFlags = [
              "-DBUILD_EXTRA=ON"
              "-DPROJECT_VERSION_FULL=${version}"
            ];

            nativeBuildInputs = [
              pkgs.buildPackages.cmake
              pkgs.buildPackages.pkg-config
            ];

            postFixup =
              (nixpkgs.lib.optionalString pkgs.stdenv.hostPlatform.isWindows ''
                mkdir -p $out/bin/
                ln -sfv ${wstdisplay.packages.${pkgs.stdenv.hostPlatform.system}.default}/bin/*.dll $out/bin/

                # FIXME: should be handled by sexpcpp itself
                ln -sfv ${pkgs.jsoncpp}/bin/*.dll $out/bin/
               '');

            buildInputs = [
              babyxml.packages.${pkgs.stdenv.hostPlatform.system}.default
            ];

            propagatedBuildInputs = [
              geomcpp.packages.${pkgs.stdenv.hostPlatform.system}.default
              logmich.packages.${pkgs.stdenv.hostPlatform.system}.default
              priocpp.packages.${pkgs.stdenv.hostPlatform.system}.default
              sexpcpp.packages.${pkgs.stdenv.hostPlatform.system}.default
              surfcpp.packages.${pkgs.stdenv.hostPlatform.system}.default
              wstdisplay.packages.${pkgs.stdenv.hostPlatform.system}.default
              wstinput.packages.${pkgs.stdenv.hostPlatform.system}.default
              wstsound.packages.${pkgs.stdenv.hostPlatform.system}.default
            ];
          };
        };
      }
    );
}
