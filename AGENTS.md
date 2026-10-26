# AGENTS.md（北航 MOS 操作系统课程实验）

## 这是什么项目
- 北航 2024 OS 课程教学内核 **MOS**：由 MIT JOS 移植到 MIPS（mipsel / MIPS32r2 / malta 板），freestanding 裸机内核（`-nostdlib`，无 glibc）。官方开源版：`github.com/buaa-os/mos.public`。
- 每个 lab 一个独立 git 分支（`lab0`~`lab6`，另有 `labX-exam`/`labX-extra` 等）。**当前分支 `lab6`**。不同实验的源码起点不一样，动手前先确认所在分支与目标 lab 一致。
- 代码沿 JOS 命名体系（`struct Env`、`page_alloc`、`env_init`、`mips_init` 等），认知背景可参考 MIT 6.828 JOS/xv6。

## 构建环境（最容易踩的坑）
- 工具链来自 Nix flake + direnv（`flake.nix` / `.envrc`）。**必须进入 devShell** 后才有两个关键环境变量：
  - `CROSS_COMPILE=mipsel-unknown-linux-gnu-`（从 nixpkgs `targetPrefix` 派生，勿硬编码）
  - `QEMU=qemu-system-mipsel` → 指向一个**包装脚本**，负责把命令行 `-cpu 4Kc` 换成 `-cpu 24Kc`
- 不进入 devShell 直接 `make` 的后果：
  - `include.mk` 默认前缀是 `mips-linux-gnu-`（本机未安装）→ 找不到交叉编译器；即使 PATH 里有 `mipsel-unknown-linux-gnu-gcc`，Makefile 也不会自动用。
  - 用裸 qemu 的 `4Kc`（MIPS32r1）跑 `-march=mips32r2` 编译的内核会触发 reserved instruction 异常，且异常处理复用 printk，导致无限递归挂死。
  - 进入方式：`direnv allow` 或 `nix develop`；临时绕过则手动 export 上述两变量（QEMU 必须用 flake 包装版）。
- **不要把 `-cpu 24Kc` 写进 Makefile / include.mk**：课程文件会被评测覆盖或比对，只能靠 flake 的 qemu 包装解决。
- `gdb-multiarch` 由 flake 提供（多架构 gdb 的包装）。

## 常用命令
- `make`（`make all`）：构建内核 `target/mos`、各模块；lab≥5 还会生成 `target/fs.img`。默认是 **debug profile**（`-O0 -g -DMOS_DEBUG`）。
- `make release` 或 `MOS_PROFILE=release make`：`-O2` + `--gc-sections`。
- `make run`：QEMU 无界面（nographic）串口运行。测试程序结束时调用 `halt()` 退出 qemu。
- `make clean` / `make clean-and-all`。
- 调试：`make dbg_run` 让 qemu 停在入口（`-s -S`，端口 1234），再 `gdb-multiarch target/mos` + `target remote localhost:1234`；`.vscode/launch.json` 已配好直接附加。
- `make objdump`：为内核与所有用户程序 `*.b` 生成反汇编 `*.objdump`。
- 构建是增量式；任何报错先看是不是忘了进 devShell（见上）。

## 实验测试机制（tests/labX_Y）
- 顶层 `lab` 变量决定当前实验：默认读 `.mos-this-lab`（内容形如 `6_0`）；命令行可覆盖 `make ... lab=2_1`。判定模块时只取第一个 `_` 前的整数（`lab-ge` 宏在 `Makefile` / `kern/include.mk` / `user/include.mk` 中大量使用）。
- 跑某个子测试：`make test lab=X_Y`。它**每次强制全量重建**（`clean-and-all`），并把 `tests/labX_Y` 注入内核：
  - 该目录的 `kernel.mk` 可声明 `init-override` / `init-envs` / `pre-env-run` / `fs-files`，经 `mk/tests.mk` 生成 `include/generated/*.h`，把 `init/init.c` 里的 `mips_init` 换成测试版（见 `init/init.c` 顶部注释）。
  - 测试 init 通常直接做 `assert(...)` 检查 + `printk` + `halt()`。
  - 目录内有 Makefile 时，其 `targets`（`.x` 用户程序）会被编进内核/盘镜像（如 lab6_1 的 testpipe 系列）。
- **`make test` 不自动启动 qemu**：烧的是测试版内核，需要再 `make run` 观察串口输出。
- **从测试状态回到普通内核必须先 `make clean`**：测试注入走编译期宏，`init.o` 已是最新时普通 `make` 不会因宏消失而重编，残留的测试 init 会被继续链接进镜像。

## 目录与构建管线
- 用户程序构建链：`.c` → 按 `user.lds` 链接成 ELF `.b` → `tools/bintoc` 转成 `.b.c`（C 字节数组）→ 编成 `.x` 打进内核或 fs 镜像（`fs/Makefile`、`user/Makefile`、`tests/include.mk` 三套同构规则）。`user/include.mk` 里 `.x` 是内嵌程序、`.b` 是放盘镜像的文件，命名有讲究。
- 目录职责：
  - `kern/` 内核主体：`entry.S`（启动）、`genex.S`/`tlb_asm.S`（异常/TLB）、`pmap.c`（内存）、`env.c`（进程）、`sched.c`、`traps.c`、`syscall_all.c`、`printk.c`。编译目标按 lab 条件增删（`kern/include.mk`）。
  - `init/`：`start.S` + `init.c`（`mips_init` 启动流程，构建时可被测试覆盖）。
  - `lib/`：内核共享库 `print.c`/`string.c`/`elfloader.c`。
  - `user/`：用户程序与用户态库（`user/include.mk` 定义 `INITAPPS/USERLIB/USERAPPS`）。
  - `fs/`：lab5+ 文件系统服务（`serv.c`/`fs.c`/`ide.c`/`check.c`），负责生成盘镜像。
  - `include/`：公共头（`asm/` 子目录；`generated/` 是构建产物）。
  - `tools/`：宿主编译的小工具 `bintoc`/`fsformat`（二进制不入库，`make` 时自动构建）+ 已入库脚本 `init-gen`、`run_bg.sh`。
  - `mk/`：顶层 make 片段 `tests.mk`（测试注入）、`profiles.mk`（debug/release）。
- `kernel.lds` 链接脚本：固定虚拟内存布局（`0x80000000` 异常入口、`0x80020000` 正文、`0x80400000` end），内含待填的练习（注释标 `/* Exercise x.y: Your code here. */`）。改布局会影响所有符号地址。
- 链接参数带 `--fatal-warnings`：链接告警会直接失败，要当真处理。

## 代码约定与工具
- 源文件注释几乎全为英文 → 新增代码注释一律用英文（本文件外的中文约定另见全局规则）。
- C 风格：`.clang-format`（LLVM 基座 + tab 缩进、4 宽、100 列）；格式化统一用 clang-format，勿手排。
- 编辑器/静态分析：
  - clangd 依赖根目录 `compile_commands.json`（`bear` 生成，已被 gitignore）。改了 include 路径或新增源文件后 IDE 报错时，用 `bear -- make <target>` 重新生成。
  - `.clangd` 已把内核子目录强制为 `--target=mipsel-unknown-linux-gnu` 并禁宿主 glibc——**不要**改成宿主 gcc/去掉 `-nostdlibinc`，否则 clangd 会注入 x86_64 头导致误报。
- 被 gitignore 的常见产物（勿提交也勿手动删）：`target/`、`include/generated/`、`*.o`、`*.b`、`*.b.c`、`*.x`、`compile_commands.json`、`.qemu_log`。

## 备注
- `tools/readelf/` 与 `tools/*.c` 中隐含字节序处理（`include.mk` 的 `CONFIG_REVERSE_ENDIAN`）：只有在非 x86 宿主上才可能触发，当前 flake 环境无需关心。
