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
- 高级模板在运行时枚举当前世界已加载的物品，自动包含基础游戏和已启用内容模组。
  原 `exotic_objects_afs` 扩展已合入基础功能，无需另行启用。
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

## 动态制造目录

高级模板使用 JSON 字段 `nanofab_template_all_items: true`。C++ 从调试物品菜单使用的
`item_controller->all()` 注册表枚举物品，排除空物品；每次打开模板制造菜单时重新读取，
无需维护物品 ID 清单。物品按本地化名称排序，保留文字搜索和分类过滤。
已启用的 Aftershock: Exoplanet 等模组自动加入目录，未启用模组不会凭空加入。
范围按物品类型 ID 枚举；同一类型的外观变体仍使用原制造过程的变体选择规则。

随机生成高级模板时也使用该目录，直接记录目标 ID，不递归构造候选物品。
普通模板默认仍使用原有的 `nanofab_template_group`，保留其随机权重与范围。
自复制模板只保留一个用途明确的物品组，指向高级模板；它不是人工维护的制造目录。
模板的材料消耗、数量限制、单次使用、修理和便携医疗功能保持不变。
已有模板保存的 `NANOFAB_ITEM_ID` 继续按原规则读取并应用物品迁移。

已删除 `Exotic_Objects_AFS` 目录及两份静态高级制造清单。
旧世界若仍列有 `exotic_objects_afs`，游戏通过核心 `mod_migration` 给出合并原因并询问
是否移除该失效扩展。确认移除后保留 `exotic_objects` 和所需内容模组即可；
不自动添加或移除 Aftershock: Exoplanet 等内容模组。

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
和自复制模板，以及动态目录的完整性、当前世界模组物品、普通模板兼容性和随机模板生成。
加载检查和定向测试命令如下（可执行文件位置随构建方式变化）：

```sh
cataclysm-tiles --userdir build/check-exotic --check-mods exotic_objects
cata_test-tiles --user-dir build/test-exotic --mods exotic_objects --option_overrides WARN_ON_MODIFIED:false "[exotic_objects]"
cata_test-tiles --user-dir build/test-exotic-afs --mods aftershock_exoplanet,exotic_objects --option_overrides WARN_ON_MODIFIED:false "[exotic_objects]"
```

动态目录验证结果（2026-09-28）：

- MSVC/CMake Release 游戏和测试程序：编译、链接成功。
- `--check-mods exotic_objects`：退出码 0，无数据加载错误。
- 基础模组：4 个测试，71365 项断言通过。
- 基础模组 + Aftershock: Exoplanet（无扩展）：4 个测试，76312 项断言通过。
- 测试逐项构造动态目录物品并检查分类，验证空 ID 排除、无重复 ID、当前加载物品全覆盖、
  Exoplanet 物品按启用状态出现、普通模板仍使用原物品组，以及 100 次随机高级模板生成。
  原有储物、模板复制、设备调用和强化突变回归测试也通过。
- 修改的 JSON 通过仓库格式器；`git diff --check` 通过。
- 中文词典可编译，新增译文格式检查通过；完整上游词典原有的 37 项严格格式诊断保持不变。

开发过程中变更 JSON 会触发缓存时间戳警告，测试目录关闭了 `WARN_ON_MODIFIED`；
数据校验本身保持开启。测试命令需要显式列出依赖，游戏的 `--check-mods` 则自动加载依赖。

验证不包含全部游戏测试、交互式菜单操作、旧存档的完整升级或声音/本地化开启的完整构建。
迁移保留了官方核心和构建方式，以上验证限制不应被理解为这些场景已实际测试。
