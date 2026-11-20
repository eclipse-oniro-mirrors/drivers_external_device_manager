# 知识库说明（docs/knowledge）

本目录为 AI 辅助生成的代码仓知识库（插件级六篇结构），是 AGENTS.md "知识库路由"的承载文档。

## 生成依据

- 基线 commit：`14f2e25`（!543 merge master into master）
- 生成日期：2026-11-10（基于 `1412c92` 初次生成）；最近刷新：2026-11-20（分支 rebase 到最新 master 后，行号与事实增量核对至 `14f2e25`，覆盖 USB/包管理行号漂移与新增 PCIE 能力）
- 生成方式：AI agent 加载 **ohos-kb-build** 技能（插件级六篇结构模板），读取目标仓真实源码后由 AI 撰写正文；全部行号引用均在生成/刷新时逐一核对
- 重跑/刷新：基线之后发生下表所列变更时，按 ohos-kb-build 技能流程对对应篇目做**增量更新**（逐篇合并、保留人工补充，不整目录重写）

## 使用优先级

- **文档与代码不一致时，一律以代码为准。**
- 文中 `file:line` 行号基于基线 commit `14f2e25`，仅作检索辅助；代码演进后允许漂移，定位时以就近的路径与符号名为准。
- 发现不一致：请提 issue 说明冲突文档与代码位置；随手修正欢迎以 PR 方式提交（仅改文档，不改代码语义）。

## 失效判定（什么改动需要刷新哪篇）

| 代码变更 | 需要刷新的篇目 |
| --- | --- |
| IDL 方法 / IPC 码（interfaces/innerkits/IDriverExtMgr.idl、hdf_ext_devmgr_interface_code.h） | architecture.md（架构约束）、coding-standards.md（接口兼容）、edm-Technology.md（SA 与 IPC 机制） |
| 错误码枚举（utils/include/edm_errors.h） | coding-standards.md、glossary.md |
| DDK C API（interfaces/ddk/、frameworks/ddk/） | coding-standards.md、edm-Technology.md（USB DDK 机制）、edm-Entity.md（DDK 传输资源） |
| 权限 / 访问控制（driver_ext_mgr.cpp 权限常量、ext_permission_manager、sa_profile） | coding-standards.md、edm-Technology.md（权限与访问控制机制） |
| 匹配 / 绑定 / 生命周期（etx_device_mgr、device、driver_extension_controller） | edm-Technology.md（匹配与生命周期机制）、edm-Entity.md（设备、绑定关系、驱动扩展连接） |
| 驱动包元数据 / RDB（drivers_pkg_manager/、pkg 表结构） | edm-Technology.md（元数据解析与持久化机制）、edm-Entity.md（驱动信息） |
| 总线扩展（bus_extension/、extdevmgr.gni 的 extdevmgr_usb_pass_through / external_device_manager_enable_pcie） | architecture.md、edm-Technology.md（总线扩展插件机制、PCIE 驱动匹配机制） |
| DFX 事件（hisysevent.yaml、drivers_hisysevent/） | coding-standards.md、edm-Technology.md（观测埋点机制） |
| 目录结构 / 构建开关（bundle.json、extdevmgr.gni、新增模块或目录） | business-context.md、architecture.md |

## 各篇分工（一句话）

- business-context.md：本组件解决什么业务问题、职责边界与上下游
- architecture.md：分层、调用链、非功能设计与架构约束
- coding-standards.md：修改代码前必须遵守的领域铁律
- glossary.md：术语与易混概念（deviceId 双编码、三套错误码等）
- edm-Entity.md：核心业务实体的概念、状态、关系与生命周期
- edm-Technology.md：核心技术机制的原理、使用方式与限制
