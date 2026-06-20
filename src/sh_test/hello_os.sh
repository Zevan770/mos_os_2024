#!/bin/bash

# 检查参数数量是否正确
if [ "$#" -ne 2 ]; then
    exit 1
fi

AAA_file="$1"
BBB_file="$2"

# 检查 AAA 文件是否存在
if [ ! -f "$AAA_file" ]; then
    exit 1
fi

# 清空 BBB 文件的内容
> "$BBB_file"

# 提取并写入
awk 'NR==8 || NR==32 || NR==128 || NR==512 || NR==1024' "$AAA_file" > "$BBB_file"
