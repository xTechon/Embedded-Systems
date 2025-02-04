{
  description = "A Nix-flake-based C/C++ development environment";

  inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";

  outputs = inputs:
    let
      system = "x86_64-linux";
      pkgs = import inputs.nixpkgs { inherit system; config.allowUnfree = true; };
      #supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      /*
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });*/

      cpp = with pkgs; [
        clang-tools
        cmake
        codespell
        cppcheck
        gtest
        gcc11
        lldb
      ];
      verilog = with pkgs; [
        iverilog # synthesizer
        svls # language server
      ];

      hw1Shell = pkgs.mkShell {
        packages = with pkgs; cpp ++ verilog ++ [
          systemc
        ];
        shellHook = ''
          export LABEL="HW1"
        '';
      };
      p1Shell = pkgs.mkShell {
        packages = cpp;
        shellHook = ''
          export LABEL="P1"
          echo $LABEL
        '';
      };

    in
    {
      devShells.${system} = {
        default = hw1Shell;
        hw1 = hw1Shell;
        p1 = p1Shell;
      };
    };
}