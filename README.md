## Embedded Systems Classwork

Classwork for my Embedded Systems Class.
<br> Uses [nix flakes](https://wiki.nixos.org/wiki/Flakes#Other_Distros,_without_Home-Manager) for consistent development environments.

`nix develop .#[assignment name]` to enter the development environment if you have [nix installed](https://github.com/DeterminateSystems/nix-installer)

Also provides VSCodium in the flake with all the required extensions for development.

- `hw1` turns a petri-net into a SystemC program and a Verilog program
- `p1` is a simulator for a petri net representing a simple processor

run `echo $LABEL` to check which development shell is active

ex:
```
> nix develop .#hw1
> echo $LABEL
HW1
> codium Embedded-Systems.code-workspace
```
Or just run `codium` in the development shell and open the workspace from there
