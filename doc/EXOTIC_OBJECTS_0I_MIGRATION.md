# Exotic Objects：0.H.D → 0.I.D 迁移记录

## 基线与范围

- 新分支：`0.I.D-branch`。
- 官方基线：`0.I-branch`，提交 `7b2efa5cea`。
- 功能来源：`origin/0.H.D-branch`，提交 `c91df7b039`。
- 采用功能移植：以官方 0.I 为基底，仅加入旧分支的有效自定义功能。
  不将 0.H 的历史回补、发布工作流、编译参数、版本号和第三方库版本覆盖到 0.I。

## 保留的功能

- `exotic_objects`：维度储物环、大容量且不增加负重的储物空间、便携纳米制造与修理、
  高级制造模板及模板复制、三种可部署机甲、强化生命值突变。
- `exotic_objects_afs`：Aftershock: Exoplanet 制造清单扩展，依赖 `exotic_objects` 和
  `aftershock_exoplanet`。0.I 已将旧 `aftershock` 标记为过时，本次以其当前后继模组为目标。
- 便携自动医疗机：CBM 安装与拆卸、骨折夹板、伤口治疗、辐射检测及治疗、血液分析。
- 中文内容：保留官方 0.I 已有翻译，只向核心中文词典补充模组的缺失译文。
  模组原有的 PO/POT 源文件也保留。

旧版没有添加自然生成途径或普通制作配方，本次也不改变获取方式。
可通过调试菜单获取储物环及自复制模板；强化突变同样保留 `valid: false`。

## 0.I 接口与数据适配

- 物品数据改为 `ITEM` + `subtypes`，价格改为带单位的金额字符串。
- 物品使用函数及手术位置改用 `tripoint_bub_ms` / `pos_bub()`。
- 强化突变的旧 `healing_*`、`mending_modifier`、`fatigue_modifier`、`stamina_regen_modifier`
  和 `cardio_multiplier` 字段已转为 0.I 附魔数值，按新乘法语义保留原单独生效时的倍率。
- 读取旧模板保存的制造编号时应用 0.I 的物品迁移表；没有有效替代品时安全拒绝制造。
- 模板白名单沿用 JSON 字段 `allowed_pocketnanofab_template_ids`，支持 0.I 的继承机制，
  并在物品定义检查中验证引用。
- 制造结果使用 0.I 的默认容器处理接口；制造菜单按当前物品分类分组并按本地化名称排序。
- 0.I 会覆盖同名物品组。Aftershock 扩展使用 `copy-from` + `extend`，不会覆盖基础清单。
- 基础清单按官方迁移记录处理了 404 个旧编号，并合并迁移到同一物品的重复项。
  基础高级制造清单保留 8771 项，Aftershock: Exoplanet 另外补充 510 项。
  清单保留旧模组原有范围，不自动纳入所有 0.I 新增物品。

基础清单移除了以下 18 个不再作为有效基础物品存在、且没有可用官方替代关系的编号：

```text
gravelbag, earthbag, bp_40x46mm_buckshot_m118, 8mm_bootleg,
artifact_teleportitis_aura, artifact_slow_aura,
manual_centipede, manual_lizard, manual_scorpion, manual_toad, manual_venom_snake,
necropolis_freq, egg_bird, deluxe_cheeseburger_wheat_free, ammo_box_army_20_308,
fn1910, ruger_redhawk, minireactor
```

其中 `minireactor` 仍保留在 Aftershock 扩展。Exoplanet 中额外移除没有替代关系的
`afs_synthetic_meat` 和 `afs_kelp`，其余改名按该模组自己的迁移表处理。
部分旧编号已成为变体或物品组，不能继续
作为独立制造物品引用；其当前基础物品在有效清单中仍然可用。

## 修复旧实现的问题

- 修理直接作用于选定物品，避免消耗另一件同型号物品；保留选定物品的状态与标识。
- 制造前检查模板内容、配方要求及材料；数量输入支持取消，限制为 1–10000，检查材料乘法溢出。
- 保留制造模板单次使用、制造物品可再次纳米修理、可变尺码护甲自动合身的行为。
- 非玩家或空角色调用便携设备时安全退出。
- 拆除 CBM 前验证麻醉剂数量；按当前手术接口保留旧模组的即时手术与高技能设定。
  不创建要求固定手术台的活动，否则便携设备的手术会中止。
- 腿部使用腿夹板；已有夹板可以继续处理；穿戴失败退回夹板。
- 单独存在破伤风时也能进入伤口治疗。
- 移除 0.I 已删除的 `FANCY` 标记，不为过时的服饰机制另加核心兼容代码。

## 构建与验证

继续使用 0.I 的 `Makefile`、CMake、Visual Studio 工程和发布工作流。
参见仓库原有 `doc/COMPILING` 与 CMake 编译说明。

唯一额外的非模组 C++ 修正是 `activity_handlers.cpp` 中对重载的
`repair_item_finish` 使用显式二参数 lambda，解决 MSVC 的重载解析错误，行为保持不变。

本机验证使用 Visual Studio 2022、已有 vcpkg 依赖和 CMake Release 图形版本，关闭声音及
本地化以缩短功能验证构建；中文 PO 另外使用 `msgfmt` 编译检查。
本机的 UTF-8、严格 C++、异常处理、大对象文件及依赖库路径适配仅放在忽略的 `build/`
目录，不改动官方构建脚本。

回归测试位于 `tests/exotic_objects_test.cpp`，包括非玩家调用、储物重量/体积、模板配方引用
和自复制模板。加载两个模组的检查和定向测试命令如下（可执行文件位置随构建方式变化）：

```sh
cataclysm-tiles --userdir build/check-exotic --check-mods exotic_objects
cataclysm-tiles --userdir build/check-exotic-afs --check-mods exotic_objects_afs
cata_test-tiles --user-dir build/test-exotic --mods exotic_objects --option_overrides WARN_ON_MODIFIED:false "[exotic_objects]"
cata_test-tiles --user-dir build/test-exotic-afs --mods aftershock_exoplanet,exotic_objects,exotic_objects_afs --option_overrides WARN_ON_MODIFIED:false "[exotic_objects]"
```

验证结果（2026-09-28）：

- MSVC/CMake Release 游戏可执行文件及测试程序：编译、链接成功。
- `--check-mods exotic_objects`：退出码 0，无数据加载错误。
- `--check-mods exotic_objects_afs`：退出码 0，无数据加载错误。
- 基础模组：3 个定向测试，17604 项断言通过。
- 基础模组 + Aftershock: Exoplanet + 扩展：3 个定向测试，18644 项断言通过。
- 测试验证了储物重量/体积、全部模板清单引用、模板复制、空角色/NPC 调用退出，
  以及强化突变的生命值、睡眠/清醒恢复倍率和心肺倍率。
- 两个模组所有 JSON 通过仓库自带格式器；`git diff --check` 通过。
- 中文词典可编译，新增译文的格式占位符检查通过。完整上游中文词典原有的 37 项严格
  `msgfmt --check-format` 诊断与未修改的 0.I 完全相同，没有改动这些无关译文。

开发过程中变更 JSON 会触发缓存时间戳警告，测试目录关闭了 `WARN_ON_MODIFIED`；
数据校验本身保持开启。测试命令需要显式列出依赖，游戏的 `--check-mods` 则自动加载依赖。

验证不包含全部游戏测试、交互式菜单操作、旧存档的完整升级或声音/本地化开启的完整构建。
迁移保留了官方核心和构建方式，以上验证限制不应被理解为这些场景已实际测试。
