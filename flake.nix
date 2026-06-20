{
  description = "MIPS 微内核开发环境 (小端, 4kc)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      # 使用官方预定义的 mipsel-linux-gnu 交叉系统（已配置小端）
      # 注意：pkgsCross 已包含正确的 gcc 和 binutils
      pkgsCross = import nixpkgs {
        system = "x86_64-linux";
        crossSystem = "mipsel-linux-gnu";
      };

      pkgs = import nixpkgs {
        system = "x86_64-linux";
      };

    in
    {
      devShells.x86_64-linux.default = pkgs.mkShell {
        name = "mips-dev";

        buildInputs = [
          pkgsCross.buildPackages.gcc # mipsel-linux-gnu-gcc
          pkgsCross.buildPackages.binutils # 包含 ld, objdump 等
          pkgs.gdb # 默认就是多架构的
          (pkgs.writeShellScriptBin "gdb-multiarch" ''
            exec ${pkgs.gdb}/bin/gdb "$@"
          '')
          pkgs.qemu # 提供 qemu-system-mipsel
          pkgs.gnumake # make
          pkgs.clang-tools
          pkgs.gcc # 主机原生 gcc（编译 host 工具）
          pkgs.binutils # 主机工具（可选）
        ];

        # 可设置环境变量，让项目 Makefile 直接使用
        shellHook = ''
          export CROSS_COMPILE=mipsel-unknown-linux-gnu-
          export QEMU=qemu-system-mipsel
        '';
      };
    };
}
