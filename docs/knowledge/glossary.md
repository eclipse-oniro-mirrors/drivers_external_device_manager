# 术语表

> 路径相对仓根；数值与枚举位均经代码核实。
> 行号基线：commit 14f2e25（2026-11-10 生成，2026-11-20 增量刷新）；与代码不一致时以代码为准，溯源与刷新规则见 [README.md](README.md)。

## 外部设备管理器（external_device_manager / EDM）
OpenHarmony HDF 子系统的组件（@ohos/external_device_manager，bundle.json:12），实现"外设驱动应用化"的框架：驱动包管理、设备-驱动匹配绑定、DDK 接口。进程内常以 EDM 缩写出现（EDM_OK、EDM_LOGI、edm_errors.h）。区别于 USB 子系统的 usb_manager（负责主机模式接口与授权），本组件不直接管理 USB 主机协议栈，而是驱动框架层。

## 系统服务 5110（SA 5110 / hdf_ext_devmgr）
本组件 SA 的编号（`HDF_EXTERNAL_DEVICE_MANAGER_SA_ID`）与进程名。由 init 以 `sa_main` + /system/profile/hdf_ext_devmgr.json 按需拉起（sa_profile/hdf_ext_devmgr.cfg:20-23），SELinux 标签 u:r:hdf_ext_devmgr:s0，APL system_basic。SA 实现类为 `DriverExtMgr`（services/native/driver_extension_manager/src/driver_ext_mgr.cpp:62）。与"5110.json"（sa_profile/5110.json，声明 libpath 与 min_hdi_proxy_version）是同一 SA 的两份配置。

## 驱动扩展（DriverExtensionAbility）
厂商 HAP 中 type 为 DRIVER 的 extension ability（ExtensionAbilityType::DRIVER），是厂商驱动的应用侧宿主。JS 形态由 `JsDriverExtension` 承载（services/native/driver_extension/src/js_driver_extension.cpp），生命周期回调 onInit/onConnect/onDisconnect/onRelease；`OnConnect` 返回的远端对象即服务侧 `Device::drvExtRemote_`。区别于普通 UIAbility：无界面、由本组件经 AbilityManagerService 以 Connect/Start 方式驱动其生死。

## 驱动包（驱动类 HAP / DRIVER HAP）
携带 DRIVER 扩展能力的应用包。module.json5 的 metadata 携带驱动声明：bus、vendor、description、launchonbind、ohos.permission.ACCESS_DDK_ALLOWED，USB 总线另有 vid、pid（逗号分隔十六进制，usb_bus_extension.cpp:190-194 解析）。由 BMS 安装，本组件订阅包事件解析入 RDB，不负责安装本身。

## 驱动包元数据行（PkgInfoTable）
持久化单元，对应 pkg.db 表 pkgInfoTable 一行（utils 侧定义：services/native/driver_extension_manager/include/drivers_pkg_manager/pkg_tables.h:22-30）：driverUid、bundleAbility、userId、appIndex、bundleName、driverName、driverInfo（序列化 JSON）。是 QueryDriverInfo 与匹配流程的共同数据源。

## 驱动唯一标识（driverUid）
驱动包内驱动的稳定身份，`driverInfo.name + "-" + accessTokenId`（drv_bundle_state_callback.cpp:157）。注意与 `DriverInfo` 构造函数中"缺省 driverUid = bundleName-driverName"（ext_object.h:44-48）的区别：持久化记录以 accessTokenId 版本为准，跨重装仍唯一；bundleName-driverName 仅是代码内缺省兜底。DDK 侧 IUsbDdk::UpdateDriverInfo 亦以此同步（usb_driver_change_callback.cpp:34/41）。

## bundleInfo（bundle 匹配键）
`ExtDeviceManager::bundleMatchMap_` 的键，格式 `bundleName + "-" + driverName`，分隔符为 `Device::stiching_`（etx_device_mgr.cpp:34，值为 "-"）。`Device::GetBundleName/GetAbilityName` 按它反拆（device.cpp:26-46）。与持久化字段 bundleAbility（bundleName-driverName，drv_bundle_state_callback.cpp:158）同构，但一个在内存匹配表、一个在 RDB。

## VID / PID（Vendor ID / Product ID）
USB 厂商/产品标识，各 16 位。匹配规则是"设备 VID ∈ 驱动 vids_ 且设备 PID ∈ 驱动 pids_"（usb_bus_extension.cpp:168-179），vids/pids 是两组独立集合，不做配对约束。设备的 VID/PID 来自设备描述符 idVendor/idProduct（usb_dev_subscriber.cpp:292-302）。

## 总线扩展（bus extension / IBusExtension）
总线类型插件抽象：`ParseDriverInfo`（解析总线专属元数据）、`MatchDriver`（总线专属匹配）、`SetDevChangeCallback`（设备事件接入）、`AcquireDriverChangeCallback`（驱动增删通知）（utils/include/ibus_extension.h:27-36）。当前唯一实现是 USB（BUS_TYPE_USB=1，ext_object.h:23-28）。区别于 HDI 总线驱动：本组件插件运行在 SA 进程内，通过 HDI 接口使用底层总线服务。区别于 PCIE 支路：PCIE 驱动匹配走独立的 PcieDriverManager（ENABLE_PCIE 门控，不经 IBusExtension/BusType 注册，见 edm-Technology.md）。

## 总线扩展核心（BusExtensionCore）
插件的装载与注册中心：dlopen /system/lib(64)/lib\<name\>.z.so（bus_extension_core.cpp:24-31、52-78），维护 busExtensions_ 注册表与 busTypeMap_（"usb"→BUS_TYPE_USB）。插件 .so 以 `__attribute__((constructor))` 自注册（usb_bus_extension.cpp:240-244）。SA 主体经它取插件，不直接耦合具体总线。

## 设备映射表（deviceMap_）
`ExtDeviceManager` 的在位设备表：`unordered_map<BusType, unordered_map<uint64_t, shared_ptr<Device>>>`（etx_device_mgr.h:74），键是 64 位 deviceId。热插拔增删、QueryDevice/QueryAllDevices 遍历都作用其上。

## 驱动匹配表（bundleMatchMap_）
`ExtDeviceManager` 的"驱动↔设备"反查表：`unordered_map<string /*bundleInfo*/, unordered_set<uint64_t> /*deviceId*/>`（etx_device_mgr.h:75）。设备命中驱动时加入（AddDevIdOfBundleInfoMap，etx_device_mgr.cpp:70）；驱动最后一个设备移除时触发 `Device::Disconnect`（RemoveDevIdOfBundleInfoMap，etx_device_mgr.cpp:119-158）；包更新/账户切换时按 bundleName 前缀批量清除再重匹配（DeleteBundlesOfBundleInfoMap，etx_device_mgr.cpp:222-242）。

## 设备标识（deviceId，64 位联合体编码）
对 JS/IPC 暴露的设备主键。`DeviceInfo::devInfo_` 联合体：高 32 位为 BusType、低 32 位为 busDeviceId（utils/include/ext_object.h:146-152）。USB 的 busDeviceId = (busNum << 16) + devAddr（usb_dev_subscriber.cpp:54-58）。**易混**：DDK C API 使用的 64 位 deviceId 是 (busNum << 32) + devAddr（ToDdkDeviceId，usb_dev_subscriber.cpp:60-63），编码规则不同，两套 ID 不可互换。

## DDK（Driver Development Kit）
面向设备厂商应用的用户态 C API 家族：USB（usb_ddk_api.h，@since 10）、Base 共享内存（ddk_api.h，@since 12）、HID（hid_ddk_api.h）、USB 串口（usb_serial_api.h）、SCSI 外设（scsi_peripheral_api.h，@since 16）。头文件在 interfaces/ddk/，实现在 frameworks/ddk/，直接以 HDI 客户端身份调用 usbd（usb_ddk_api.cpp:96 `IUsbDdk::Get()`），不经 SA 5110。权限 ohos.permission.ACCESS_DDK_USB 在 HDI 服务侧校验。

## USB 直通开关（extdevmgr_usb_pass_through）
extdevmgr.gni:20 构建开关（默认 true）。true：总线扩展使用 HDI 代理 2.0 `IUsbHostInterface`（服务名 "usb_host_interface_service"，usb_bus_extension.cpp:34、96-104）；false：使用 1.0 `IUsbInterface` 并挂 usbd 死亡监听（卸载自身，usb_bus_extension.cpp:105-119、219-233）。影响测试 mock 选择（AGENTS.md 亦注明）。

## 绑定即拉起（launchonbind）
驱动包元数据开关（drv_bundle_state_callback.cpp:52/96-98，持久化字段 launch_on_bind，driver_info.cpp:43）。为 true 时，设备匹配不立即 `Device::Connect()`（etx_device_mgr.cpp:103-106），改为应用 bindDevice 时拉起；解绑时若为 true 且调用方是最后一个 caller 才真正断开（etx_device_mgr.cpp:594-601）。默认 false（设备在位即拉起驱动）。

## 驱动可访问开关（ohos.permission.ACCESS_DDK_ALLOWED / accessAllowed）
驱动包元数据声明（drv_bundle_state_callback.cpp:53/99-101），决定其余应用能否通过 bindDriverWithDeviceId 绑定该驱动：`DriverInfo::IsAccessAllowed()` 为 false 直接拒绝（EDM_ERR_SERVICE_NOT_ALLOW_ACCESS，etx_device_mgr.cpp:677-680）。

## 驱动访问授权名单（ACCESS_DDK_DRIVERS 的 appIds）
`ohos.permission.ACCESS_DDK_DRIVERS` 权限的可用性参数，JSON 形如 {"appIds":"id1,id2"}。服务端经 `ExtPermissionManager::GetPermissionValues` 解析为集合（ext_permission_manager.cpp:74-104），与驱动应用 provision 的 `appIdentifier` 比对（etx_device_mgr.cpp:707-718）；驱动应用自身（appIdentifier 相同）自动放行（IsCallerMatchDriverApp，etx_device_mgr.cpp:721-737）。是 bindDriverWithDeviceId 区别于 bindDevice 的核心授权链。

## 应用身份标识（appIdentifier）
provision 文件中的应用身份（AppProvisionInfo.appIdentifier），由 BMS `GetAppProvisionInfo` 获取。用于跨 bundle 判"是否同一驱动应用"；比 bundleName 更稳（重装不变）。

## 错误码体系（UsbErrCode / ErrCodeOffset）
`EDM_OK=0`，其余 `ErrCodeOffset(SUBSYS_DRIVERS, EDM_MODULE_ID=1)` 偏移（utils/include/edm_errors.h:23-48）。**易混**：DDK 返回码（USB_DDK_SUCCESS=0 等，interfaces/ddk/usb/usb_ddk_types.h）与 JS 错误码（201/202/401/22900001/26300001-03，device_manager_middle.h:27-35）是三套独立体系，分别用于 SA 内部、DDK C API、NAPI 层。

## 观测事件域（EXTERNAL_DEVICE）
HiSysEvent 域，含 5 个 STATISTIC 事件（hisysevent.yaml:14-74）：DRIVER_PACKAGE_CYCLE_MANAGER（包生命周期）、EXT_DEVICE_EVENT（设备/绑定，CRITICAL）、PCIE_MATCH_EVENT（PCIE 驱动匹配）、EXTERNAL_DEVICE_SA_EVENT（SA 事件）、EXTERNAL_DEVICE_DDK_EVENT（DDK 调用）。字段 schema 是对外契约。

## 驱动包数据库（pkg.db）
RDB 数据库，路径 /data/service/el1/public/pkg_service/pkg.db（pkg_database.h:31-33，目录由 sa_profile init job 创建），表 pkgInfoTable，安全等级 S1（pkg_database.cpp:32）。承载 PkgInfoTable 行，是匹配与查询的持久数据源；跨重启保留。

## 模拟器降级（EmulatorNotSupported）
`is_emulator` 构建宏下（extdevmgr.gni:40-42 → IS_EMULATOR），DDK/NAPI 入口统一返回 DDK_EMULATOR_NOT_SUPPORT（utils/include/ddk_emulator_utils.h；usb_ddk_api.cpp:83 等各入口前置守卫）。属产品形态裁剪，不是错误处理。

## 按需服务（on-demand SA）
本 SA 声明 `"ondemand": true`（hdf_ext_devmgr.cfg:22）：IPC 请求到达才拉起；设备全部移除后 `UnLoadSelf` 经 30 秒 Utils::Timer 再 `UnloadSystemAbility(5110)`（etx_device_mgr.cpp:33/499-537）。配合 samgr 的 Load/UnloadSystemAbility 使用；客户端用 CheckSystemAbility 而非 GetSystemAbility 触发（driver_ext_mgr_client.cpp:74）。ENABLE_PCIE 构建下 PCIE 设备在位会阻止自卸载（SetPcieDevicesPresent，etx_device_mgr.cpp:486-497）。
