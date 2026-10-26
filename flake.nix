{
  description = "MIPS 微内核开发环境 (小端, mips32r2)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";

      # 不用 pkgsCross.<target>.mkShell：会把整个 mipsel sysroot（几百个包、
      # 基本无缓存）拉进闭包，从源码构建数小时。
      # 本项目是 freestanding 内核（-nostdlib），编译链接都无需 glibc，
      # 只要交叉 gcc/binutils，故用宿主 mkShell 轻量接入即可。
      pkgsCross = import nixpkgs {
        inherit system;
        crossSystem = "mipsel-linux-gnu";
      };

      # 宿主（x86_64）实例：devShell 本体 + 所有构建期工具。
      pkgs = import nixpkgs {
        inherit system;
      };

      # 真实安装的交叉工具链前缀（实测为 mipsel-unknown-linux-gnu-）。
      # 从 targetPrefix 派生而不是硬编码，避免三元组命名漂移：
      # Debian 的交叉工具链叫 mipsel-linux-gnu-gcc，nixpkgs 归一化为
      # mipsel-unknown-linux-gnu-gcc —— 是同一个三元组的不同写法。
      crossPrefix = pkgsCross.buildPackages.gcc.targetPrefix;

      # 注：内核用 -march=4kc 编译，与 qemu 默认 -cpu 4Kc（MIPS32r1 核）一致，
      # 无需任何 -cpu hack，QEMU 直接指向真实 qemu-system-mipsel。

    in
    {
      devShells.${system}.default = pkgs.mkShell {
        name = "mips-dev";

        # packages == nativeBuildInputs：构建期工具（mkShell 现代推荐写法）。
        # 交叉工具链取 pkgsCross.buildPackages.*（跑在 x86_64、产出 mipsel 代码）。
        packages = [
          pkgsCross.buildPackages.gcc # mipsel-unknown-linux-gnu-gcc
          pkgsCross.buildPackages.binutils # 包含 ld, objdump 等
          pkgs.gdb # 默认就是多架构的
          (pkgs.writeShellScriptBin "gdb-multiarch" ''
            exec ${pkgs.gdb}/bin/gdb "$@"
          '')
          pkgs.qemu # 提供 qemu-system-mipsel
          pkgs.gnumake # make
          pkgs.bear
          pkgs.clang-tools # clangd/clang-format 供 IDE
        ];

        # buildInputs：目标平台（mipsel）库。
        # 本项目是 freestanding 裸机内核（-nostdlib），不需要任何目标平台库，留空。
        # 注：include.mk 的 HOST_CC := cc 依赖系统自带的宿主编译器（Debian 自带）。

        # 可设置环境变量，让项目 Makefile 直接使用
        shellHook = ''
          # Makefile 用 $(CROSS_COMPILE)gcc / $(CROSS_COMPILE)ld 调工具链，
          # 前缀必须与实际安装的二进制一致，故由 targetPrefix 派生而非硬编码。
          export CROSS_COMPILE=${crossPrefix}
          export QEMU=qemu-system-mipsel
        '';
      };
    };
}
