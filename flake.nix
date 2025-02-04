{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";
    flake-utils.follows = "nix-vscode-extensions/flake-utils";
    nix-vscode-extensions.url = "github:nix-community/nix-vscode-extensions";
  };

  outputs = inputs:
    let
      system = "x86_64-linux";
      pkgs = import inputs.nixpkgs { inherit system; config.allowUnfree = true; };
      extensions = inputs.nix-vscode-extensions.extensions.${system};
      #supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      /*
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });*/
      
      # run with `nix develop .#codium`
      packages.${system}.codium = pkgs.vscode-with-extensions.override {
          vscode = pkgs.vscodium;
          vscodeExtensions = [
            extensions.vscode-marketplace.dalance.svls-vscode  
            extensions.open-vsx.bbenoist.nix
            extensions.open-vsx.jnoortheen.nix-ide
            extensions.open-vsx.asvetliakov.vscode-neovim
            extensions.open-vsx.xaver.clang-format
            extensions.open-vsx.llvm-vs-code-extensions.vscode-clangd
            extensions.open-vsx.vadimcn.vscode-lldb
          ];
        };


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
        packages = cpp ++ verilog ++ [
          pkgs.systemc
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
      inherit packages;
    };
}
