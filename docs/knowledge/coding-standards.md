# 编码铁律

> 依据本仓现行代码与 AGENTS.md 约束整理，路径相对仓根。规则均给出代码依据。

## 并发模型

- **禁止**在 `ExtDeviceManager` 的 `deviceMap_`/`bundleMatchMap_` 上无锁读写，**采用** `std::lock_guard<std::mutex>` 分别持 `deviceMapMutex_`/`bundleMatchMapMutex_`（etx_device_mgr.h:72-75）。IPC 线程、HDI 热插拔回调、包事件派发线程、定时器线程并发触达这两张表；`ClearMatchedDrivers` 演示了先 deviceMap 锁再 bundleMatchMap 锁的固定顺序（etx_device_mgr.cpp:280-311），改序有死锁风险。
- **禁止**跨进程持有裸指针，**采用** sptr/wptr 托管 `IRemoteObject`，并以 `DeathRecipient` 感知对端死亡：`DriverExtMgrCallbackDeathRecipient`（device.cpp:188-197、285-295）、`DriverExtMgrDeathRecipient`（driver_ext_mgr_client.cpp:45-53）、`BundleMgrDeathRecipient`（drv_bundle_state_callback.cpp:518-526）、非 pass-through 模式 `UsbdDeathRecipient`（usb_bus_extension.cpp:219-233）是四类必需的死亡清理。
- **禁止**在锁内调用可能回锁的外部 IPC，**采用**`Device::deviceMutex_` 用 recursive_mutex 吸收回调重入（device.h:179），`GetTotalDeviceNum` 明示不加锁并在持锁调用方执行（etx_device_mgr.cpp:471-484 注释）。
- DDK 侧生命周期串行化：`g_lifecycleMutex`（Init/Release 互斥）+ `g_mutex`（shared_mutex 保护 g_ddk）+ `g_fdMapMutex`（fd/缓冲表），见 usb_ddk_api.cpp:51-53；读路径用 `std::shared_lock`。

## 错误处理

- **禁止**自造错误码数值，**采用** `UsbErrCode`（utils/include/edm_errors.h:27-48）：`EDM_OK = 0`，其余全部 `ErrCodeOffset(SUBSYS_DRIVERS, EDM_MODULE_ID)` 偏移；**禁止**在既有枚举值前插入新值（后值全部漂移）。DDK C API 用 `UsbDdkErrCode`/`DDK_RetCode`（interfaces/ddk/*/\<bus\>_types.h），不是 UsbErrCode。
- **禁止**吞掉 IPC 失败细节，**采用**双通道返回：proxyRet（IPC 层）+ errorCode（业务层）。服务端方法统一 `return EDM_OK`（表示 IPC 成功），业务结果写 errorCode（见 driver_ext_mgr.cpp:136-426 各方法）；客户端 `ProxyRetTranslate` 把 ERR_INVALID_VALUE/ERR_INVALID_DATA 映射为 EDM_ERR_INVALID_OBJECT/EDM_ERR_INVALID_PARAM（driver_ext_mgr_client.cpp:42-58）。
- **禁止**在 NAPI 层直接透传内部错误码，**采用** device_manager_middle.h:27-35 的对外码：201（无权限）/202（非系统应用）/401（参数）/22900001（服务异常）/26300001/26300002（不允许访问）/26300003（无绑定关系）。
- **禁止**未初始化即调用 DDK，**采用**先 `OH_Usb_Init`；未初始化/服务关闭统一返回 `USB_DDK_INVALID_OPERATION`（usb_ddk_api.cpp 全部入口的 g_ddk 判空模式）。
- JSON 解析必须判空并逐字段验型后使用（driver_info.cpp:55-103 的 `IsJsonObjValid`/`GetBoolValue`/`GetStringValue` 模式），失败返回 `EDM_ERR_JSON_PARSE_FAIL`/`EDM_ERR_JSON_OBJ_ERR`。
- 使用 `memcpy_s`/`sprintf_s` 而非裸 memcpy/sprintf（usb_ddk_api.cpp:394、usb_dev_subscriber.cpp:45）。

## 接口兼容（最高优先级）

- **禁止**修改 IDL 方法签名或重排 `DriverExtMgrInterfaceCode` 1-8（hdf_ext_devmgr_interface_code.h:23-30）；**禁止**手改生成物（stub/proxy、frameworks/js/ani、frameworks/js/taihe）。
- **禁止**修改 DDK C API 的签名、参数类型、返回码值与 @since 版本（usb_ddk_api.h @since 10、ddk_api.h @since 12、scsi_peripheral_api.h @since 16 等公共版本化契约）。
- **禁止**变更 hisysevent.yaml 中 4 个事件的字段名/类型/等级（DRIVER_PACKAGE_CYCLE_MANAGER、EXT_DEVICE_EVENT、EXTERNAL_DEVICE_SA_EVENT、EXTERNAL_DEVICE_DDK_EVENT，hisysevent.yaml:16-63）——下游观测依赖该 schema。
- **禁止**变更 sa_profile 的 UID/GID/APL/SELinux 标签与 5110.json 的 `min_hdi_proxy_version`。

## 权限与安全

- **禁止**移除或放宽入口校验：`BindDevice/UnBindDevice/QueryDevice/QueryDeviceInfo/QueryDriverInfo` 必须先 `ExtPermissionManager::VerifyPermission("ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER")`（driver_ext_mgr.cpp:41、139-147）；`BindDriverWithDeviceId/UnBindDriverWithDeviceId` 用 `ohos.permission.ACCESS_DDK_DRIVERS` 且其 appIds 参数参与 `CheckAccessPermission` 的 appIdentifier 白名单比对（driver_ext_mgr.cpp:42、215-230，etx_device_mgr.cpp:651-700）。
- **禁止**对普通三方应用开放系统接口：`QueryDeviceInfo/QueryDriverInfo/NotifyUsbPeripheralFault` 先 `IsSystemAppOrSa()`（driver_ext_mgr.cpp:337/369/402）。
- **禁止**在日志明文输出敏感值（序列号、token、权限值）；埋点中 SN_NUM 只取末 5 位（driver_report_sys_event.cpp:29、70-72）；日志格式用 `%{public}s`/`%{private}s` 显式区分（hilog_wrapper.h 宏约定）。

## 日志规范

- **统一采用** `EDM_LOGF/LOGE/LOGW/LOGI/LOGD(module, fmt, ...)`（utils/include/hilog_wrapper.h:116-130），module 取 `UsbMgrSubModule` 枚举（0xD002550 域，hilog_wrapper.h:60-75）；**禁止**散用 printf/cout（drv_bundle_state_callback.cpp:200 的 cout 仅存于历史测试辅助路径 PrintTest，勿仿写）。

## 命名与文件

- 文件 snake_case；头文件守护宏 `OHOS_EXTERNAL_DEVICE_MANAGER_<FILENAME>_H`（device.h:16）；命名空间双层 `OHOS::ExternalDeviceManager`；类/方法 PascalCase、成员 camelCase_、常量 UPPER_SNAKE（见各现行头文件）。
- 新文件必须带 Apache 2.0 许可头；**禁止**删除既有许可头。

## 构建与依赖

- **禁止**在 BUILD.gn 增加 bundle.json 未声明的外部依赖（bundle.json deps.components 是 CI 校验面）；构建行为开关集中在 extdevmgr.gni `declare_args`（extdevmgr.gni:19-29），**禁止**在子目录 BUILD.gn 私设同名开关。
- 变更 USB 代理路径时注意 `extdevmgr_usb_pass_through` 影响：测试 mock 随之切换 usb_host_impl_mock.cpp / usb_impl_mock.cpp（test/unittest/bus_extension_usb_test/）。
- 提交前对改动文件执行 `clang-format -i`（.clang-format：WebKit 基底，ColumnLimit 120，PointerAlignment Right）。

## 测试要求

- 新增/修改逻辑须同步 test/unittest（组目标 external_device_manager_ut，25 个既有用例目录）、必要时补 test/moduletest 与 test/fuzztest（含 idriver_ext_mgr_callback stub 的模糊入口）；单测组受 `external_device_manager_enable_service` 门控（test/unittest/BUILD.gn）。
- 构建与测试命令见 AGENTS.md「Build Commands / Test Commands」；本仓在 Windows 开发环境下无法运行 OpenHarmony 工具链时，须在交付说明中明确"未执行"。
