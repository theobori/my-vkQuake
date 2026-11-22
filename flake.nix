{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells = {
          default = pkgs.mkShell {
            packages = with pkgs; [
              glslang
              spirv-tools
              meson
              ninja
              pkg-config
              sdl3
              flac
              gzip
              libmpg123
              libopus
              libvorbis
              libx11
              opusfile
              vulkan-loader
              llvmPackages_18.clang-tools
            ]
            ++ lib.optionals stdenv.hostPlatform.isDarwin [
              moltenvk
              vulkan-headers
            ];
          };
        };
      }
    );
}
