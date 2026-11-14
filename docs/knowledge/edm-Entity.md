# 业务实体知识

external_device_manager 模块核心业务实体的完整知识定义，涵盖静态概念、动态运行、规则约束与代码映射。路径相对仓根。

> 行号基线：commit 1412c92（2026-11-10 生成）；与代码不一致时以代码为准，溯源与刷新规则见 [README.md](README.md)。

## 外部设备（Device / DeviceInfo / UsbDeviceInfo）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 外部设备（Device，运行时对象）；DeviceInfo/UsbDeviceInfo（总线侧数据） |
| 实体定义 | 一个物理在位的外设及其服务侧管理记录：设备描述数据 + 匹配到的驱动信息 + 绑定状态。Device 是"被管理设备"的聚合根，DeviceInfo 只描述硬件身份 |
| 核心特征 | 64 位 deviceId（busType 高 32 位 + busDeviceId 低 32 位，utils/include/ext_object.h:146-152）；UsbDeviceInfo 携带 bcdUSB/idVendor/idProduct/deviceClass/deviceSubClass/deviceProtocol/snNum/interfaceDescList_（services/native/driver_extension_manager/include/bus_extension/usb/usb_device_info.h:63-69）；Device 上 `bundleInfo_` 非空即 HasDriver()（services/native/driver_extension_manager/include/device_manager/device.h:43-46） |
| 类型/分类 | BusType 分类（BUS_TYPE_USB=1 当前唯一生产值，ext_object.h:23-28） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 创建：热插拔上线 → `UsbDevSubscriber::OnDeviceConnect` 读描述符构造 UsbDeviceInfo（services/native/driver_extension_manager/src/bus_extension/usb/usb_dev_subscriber.cpp:161-214）→ `DevChangeCallback::OnDeviceAdd` → `ExtDeviceManager::RegisterDevice` 创建/复用 Device 并匹配驱动（services/native/driver_extension_manager/src/device_manager/etx_device_mgr.cpp:315-374）。更新：绑定/解绑改其 boundCallerInfos_/drvExtRemote_。移除：下线 → `UnRegisterDevice`（etx_device_mgr.cpp:376-415），若已有驱动连接则先 UnRegist 标记、待 OnDisconnect 后从 deviceMap_ 摘除（device.cpp:215-217） |
| 状态流转 | 关键标记：`isUnRegisted`（设备已拔出但驱动连接尚未收尾，device.h:123-131）；`bundleInfo_` 空↔非空（无驱动↔有驱动）；绑定 caller 集合从空到非空（CallerInfo::isBound 表示驱动远端是否已就绪，device.h:30-32、130）。合法变化：上线（无驱动）→命中驱动→Connect→isBound=true→解绑/拔出→清理 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 根 HUB 不注册（bDeviceClass==0x09 且 devAddr==0x01，usb_dev_subscriber.cpp:39-40、188-192）；已注册且已有 drvExtRemote_ 的重复上线事件直接幂等返回 EDM_OK（etx_device_mgr.cpp:326-329）；未匹配驱动的设备保留在表中等待驱动包安装（etx_device_mgr.cpp:357-362） |
| 并发约束 | deviceMap_ 操作须持 deviceMapMutex_（etx_device_mgr.cpp:320/383）；Device 内部状态用 recursive deviceMutex_ + drvExtRemoteMutex_（device.h:179-180）；GetTotalDeviceNum 刻意不加锁、依赖调用方持锁（etx_device_mgr.cpp:471-484 注释） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | ExtDeviceManager::deviceMap_ 按 (busType, deviceId) 持有 shared_ptr\<Device\>（etx_device_mgr.h:72）；bundleMatchMap_ 反向按 bundleInfo 聚合 deviceId |
| 下游影响 | 绑定状态驱动 IDriverExtMgrCallback 回调与 AMS 侧 Ability 生死；查询 API（USBDevice/DeviceInfoData）由其装配（driver_ext_mgr.cpp:151-173、247-293） |
| 平级关联 | DeviceInfo 由总线扩展生产（UsbDevSubscriber）；DriverInfo 经 AddBundleInfo 挂接（device.h:58-65） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `Device`（services/native/driver_extension_manager/src/device_manager/device.cpp）、`DeviceInfo`/`UsbDeviceInfo`（utils/include/ext_object.h:117、services/native/driver_extension_manager/include/bus_extension/usb/usb_device_info.h） |
| 核心方法 | `RegisterDevice`/`UnRegisterDevice`（etx_device_mgr.cpp:315/376）、`Device::Connect`（无参/带回调两个重载，device.cpp:61/96）、`Device::Disconnect`（device.cpp:145）、`QueryDeviceByDeviceID`（etx_device_mgr.cpp:521） |

## 驱动信息（DriverInfo / UsbDriverInfo 与 PkgInfoTable）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 驱动信息（DriverInfo）；总线扩展部分（DriverInfoExt/UsbDriverInfo）；持久化形态（PkgInfoTable） |
| 实体定义 | 一个厂商驱动的完整画像：身份（bundleName/driverName/driverUid）、归属（userId/bus）、展示（vendor/version/description/driverSize）、行为开关（launchOnBind/accessAllowed）、总线匹配条件（vids_/pids_）。持久化行是其在 pkg.db 的物化 |
| 核心特征 | DriverInfo 字段组（utils/include/ext_object.h:100-114）；UsbDriverInfo 仅 pids_/vids_ 两个 uint16 向量（services/native/driver_extension_manager/include/bus_extension/usb/usb_driver_info.h:36-39）；序列化 JSON 键：bus/vendor/version/size/description/ext_info/launch_on_bind/access_allowed（services/native/driver_extension_manager/src/drivers_pkg_manager/driver_info.cpp:37-44），ext_info 内嵌 {"pids":[],"vids":[]}（services/native/driver_extension_manager/src/bus_extension/usb/usb_driver_info.cpp:87-91） |
| 类型/分类 | 按 bus 字符串分派总线插件解析 ext_info（driver_info.cpp:123-135） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 产生：BMS 扫描 DRIVER 扩展 → `DrvBundleStateCallback::ParseToPkgInfoTables`（drv_bundle_state_callback.cpp:105-152，`CreatePkgInfoTable` 生成 driverUid=name+"-"+accessTokenId）→ `UpdateToRdb` 落库（drv_bundle_state_callback.cpp:456-481）。消费：`DriverPkgManager::QueryMatchDriver` 读库逐行 UnSerialize 后交总线插件 MatchDriver（driver_pkg_manager.cpp:114-159）；`QueryDriverInfo` 直接整表/按 driverUid 返回（driver_pkg_manager.cpp:161-192）。消亡：包删除/元数据清空 → `OnBundleDrvRemoved` 删行（drv_bundle_state_callback.cpp:531-554） |
| 状态流转 | RDB 行随包事件 replace（AddOrUpdatePkgInfo，pkg_db_helper.h:53）；内存中每次匹配重建临时对象，无长期驻留状态。匹配成功时镜像到 Device（bundleInfo_/driverUid_）并在 bundleMatchMap_ 建立反查 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 元数据键大小写：bus/vendor/description 精确匹配，launchonbind 忽略大小写（LowerStr，drv_bundle_state_callback.cpp:96）；vid/pid 值按十六进制解析（usb_bus_extension.cpp:199-217）；driverUid 持久层以 accessTokenId 版本为准（drv_bundle_state_callback.cpp:157）；包事件仅处理当前激活用户的（IsCurrentUserId，drv_bundle_state_callback.cpp:494-497） |
| 并发约束 | BMS 代理访问持 bundleMgrMutex_；initOnce 保证启动全量扫描一次（drv_bundle_state_callback.cpp:316-345）；RDB 读写经 PkgDbHelper::databaseMutex_（pkg_db_helper.h:68） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | ExtDeviceManager 匹配流程持有临时 DriverInfo；Device::driverInfo_ 保存命中结果（device.h:58-65） |
| 下游影响 | 命中后驱动 UsbDriverChangeCallback → IUsbDdk::UpdateDriverInfo（usb_driver_change_callback.cpp:23-42）；QueryDriverInfo API 输出 DriverInfoData（driver_ext_mgr.cpp:295-331） |
| 平级关联 | 与 DeviceInfo 通过总线插件 MatchDriver 发生判定关系；与 bundleInfo 一一对应（bundleName-driverName） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `DriverInfo`/`DriverInfoExt`（utils/include/ext_object.h:31-115）、`UsbDriverInfo`（services/native/driver_extension_manager/include/bus_extension/usb/usb_driver_info.h）、`PkgInfoTable`（services/native/driver_extension_manager/include/drivers_pkg_manager/pkg_tables.h:22-30） |
| 核心方法 | `ParseToPkgInfoTables`/`CreatePkgInfoTable`/`UpdateToRdb`（drv_bundle_state_callback.cpp:105/154/456）、`DriverInfo::Serialize/UnSerialize`（driver_info.cpp:24/105）、`PkgDbHelper::AddOrUpdatePkgInfo/QueryPkgInfos/DeleteRightRecord`（pkg_db_helper.h:39-53） |

## 绑定关系（Device 上的 caller/callback/远端对象三元组）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 应用-驱动绑定关系（boundCallerInfos_ + callbacks_ + drvExtRemote_） |
| 实体定义 | 一个 Device 上"哪些应用绑了它、各自的回调是谁、驱动远端对象是否就绪"的完整记录。是 BindDevice/UnBindDevice 语义的运行时载体 |
| 核心特征 | `boundCallerInfos_`：unordered_map\<uint32_t tokenId, CallerInfo{isBound}\>（device.h:185）；`callbacks_`：按 AsObject() 排序去重的 sptr\<IDriverExtMgrCallback\> 集合（device.h:158-169/182）；`drvExtRemote_`：sptr\<IRemoteObject\> 驱动远端对象（device.h:181） |
| 类型/分类 | 不区分总线；同一 Device 可多 caller 并存 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 建立：bindDevice → `Device::Connect(callback, tokenId)`：drvExtRemote_ 已存在则立即 OnConnect 回调并置 isBound=true（device.cpp:102-115）；否则登记 caller={isBound:false}、注册回调与死亡监听、发起 ConnectAbility（device.cpp:117-138）。驱动就绪：`OnConnect` 统一置全部 caller isBound=true 并广播（device.cpp:177-195）。解除：unbindDevice 非最后 caller 只 `RemoveCaller`（etx_device_mgr.cpp:575-582）；最后 caller 或拔出触发 Disconnect → `OnDisconnect` 清空三者并 StopDriverExtension（device.cpp:197-222）。caller 进程死亡：`DriverExtMgrCallbackDeathRecipient::OnRemoteDied` 仅移除回调（device.cpp:285-295） |
| 状态流转 | 空 → {caller:isBound=false} → （驱动连上）全体 isBound=true → 逐个解绑 → 空+Stop；拔出时先 isUnRegisted=true，OnDisconnect 后从 deviceMap_ 摘除（device.cpp:215-217） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 真正断开驱动的条件：launchOnBind 为 true 且调用方是最后 caller（IsLastCaller，etx_device_mgr.cpp:575/636）；launchOnBind 为 false 时解绑仅移除 caller、驱动随设备在位存活；UnBindDriverWithDeviceId 校验存在绑定关系否则 EDM_ERR_SERVICE_NOT_BOUND（etx_device_mgr.cpp:621-626） |
| 并发约束 | 全部变更持 Device::deviceMutex_（recursive）；callbacks_ 修改与遍历同锁；远端对象读写另有 drvExtRemoteMutex_（device.h:93-109） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | 附着于 Device；caller 身份来自 `ExtPermissionManager::GetCallingTokenID()`（IPC 骨架取值，ext_permission_manager.cpp:69-72） |
| 下游影响 | OnDisconnect 触发 AMS StopExtensionAbility（device.cpp:221）；驱动远端对象经回调交付应用后由应用侧使用（NAPI 转成 JS 对象，device_manager_middle.cpp） |
| 平级关联 | NAPI 层 g_callbackMap\<deviceId, AsyncData\> 与之镜像（device_manager_middle.cpp:55） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `Device`（成员组）、`CallerInfo`（device.h:30-32）、`DriverExtMgrCallbackDeathRecipient`（device.h:188-197） |
| 核心方法 | `Connect`/`Disconnect`/`OnConnect`/`OnDisconnect`/`RemoveCaller`/`IsLastCaller`/`IsBindCaller`/`RegisterDrvExtMgrCallback`/`RegisteDeathRecipient`（device.cpp 全文） |

## 驱动扩展连接（DrvExtConnectionInfo / DriverExtensionAbilityConnection / DrvExtConnNotify）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 驱动扩展连接（一次 AMS ConnectAbility 的会话记录） |
| 实体定义 | Device 到厂商 DriverExtensionAbility 的连接凭据：目标（bundleName/abilityName/deviceId）、AMS 侧连接桩（持有 remoteObject_）、以及服务侧通知器（DrvExtConnNotify） |
| 核心特征 | `DrvExtConnectionInfo`（services/native/driver_extension_manager/include/device_manager/driver_extension_controller.h:51-56）；`DriverExtensionAbilityConnection : AbilityConnectionStub`（services/native/driver_extension_manager/src/device_manager/driver_extension_controller.cpp:26-56，callback_ 为 weak_ptr）；`DrvExtConnNotify : IDriverExtensionConnectCallback`（services/native/driver_extension_manager/include/device_manager/device.h:199-216，持 weak_ptr\<Device\>） |
| 类型/分类 | 每次 ConnectDriverExtension 生成一组，不跨 Device 复用 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `ConnectDriverExtension` 创建三元组并 `ConnectAbility(want, connectInner_, -1)`，Want 携带 deviceId 参数（driver_extension_controller.cpp:131-149）；AMS 回 `OnAbilityConnectDone` 存 remoteObject_ 并回调 OnConnectDone → Device::OnConnect；断开时校验三元组一致后 `DisconnectAbility`（driver_extension_controller.cpp:154-189），AMS 回 OnAbilityDisconnectDone → Device::OnDisconnect |
| 状态流转 | 未连接 → ConnectAbility 已发起 → remoteObject_ 非空（IsConnectDone，driver_extension_controller.h:63-66）→ 断开 remoteObject_ 置空并触发 Device 清理。失败重试：无参 Connect 循环最多 30 次×100ms（device.cpp:73-89，针对开机 BMS 未就绪的 RESOLVE_ABILITY_ERR） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 同一 IDriverExtensionConnectCallback 对象 info_ 已存在则拒绝复用（EDM_ERR_INVALID_OBJECT，driver_extension_controller.cpp:126-129）；Disconnect 校验 bundleName/abilityName/deviceId 三元一致（driver_extension_controller.cpp:171-176） |
| 并发约束 | 回调链均为 sptr/weak_ptr 托管；DrvExtConnNotify 持 weak_ptr 防 Device 提前释放悬挂（device.h:201） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | Device::connectNofitier_ 持有 DrvExtConnNotify（device.h:183）；驱动就绪后 remoteObject_ 即 Device::drvExtRemote_ |
| 下游影响 | 对端是厂商 JsDriverExtension::OnConnect 返回的远端对象（services/native/driver_extension/src/js_driver_extension.cpp:267-321） |
| 平级关联 | StartDriverExtension/StopDriverExtension 不建连接、仅控制 Ability 进程（driver_extension_controller.cpp:68-112） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `DriverExtensionController`（单例）、`DriverExtensionController::DriverExtensionAbilityConnection`、`IDriverExtensionConnectCallback`（services/native/driver_extension_manager/include/device_manager/driver_extension_controller.h） |
| 核心方法 | `ConnectDriverExtension`/`DisconnectDriverExtension`/`StartDriverExtension`/`StopDriverExtension`（services/native/driver_extension_manager/src/device_manager/driver_extension_controller.cpp:68/113/154/91） |

## DDK 传输资源（UsbDeviceMemMap / DDK_Ashmem / interfaceHandle / g_fdMap）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | DDK 数据传输资源（接口句柄、设备内存映射、共享内存） |
| 实体定义 | 应用进程内由 DDK 分配、随传输请求复用的内核共享资源句柄：interfaceHandle（claimed 接口）、UsbDeviceMemMap（mmap 缓冲）、DDK_Ashmem（匿名共享内存）、UsbRequestPipe（端点管道描述） |
| 核心特征 | UsbDeviceMemMap{address,size,offset,bufferLength,transferedLength}（interfaces/ddk/usb/usb_ddk_types.h）；DDK_Ashmem 含 ashmemFd（interfaces/ddk/base/ddk_types.h）；进程级登记表 g_fdMap\<buffer*, fd\> 与 g_bufferSizeMap（frameworks/ddk/usb/usb_ddk_api.cpp:50-56） |
| 类型/分类 | 仅存在于调用方进程；DDK 库与 usbd 经 fd 共享，不经 SA 5110 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 创建：`OH_Usb_ClaimInterface`（HDI 返回 interfaceHandle，usb_ddk_api.cpp:282-302）；`OH_Usb_CreateDeviceMemMap`：GetDeviceMemMapFd + ftruncate + mmap，登记 g_fdMap（usb_ddk_api.cpp:497-545）；`OH_DDK_CreateAshmem`（frameworks/ddk/base/）。使用：`OH_Usb_SendPipeRequest(pipe, devMmap)` 校验 offset/bufferLength 后整段交 HDI（usb_ddk_api.cpp:431-461）；`OH_Usb_SendPipeRequestWithAshmem` 携带 fd 与地址向量（usb_ddk_api.cpp:463-495）。释放：`OH_Usb_ReleaseInterface`/`OH_Usb_DestroyDeviceMemMap`（munmap+close fd+erase，usb_ddk_api.cpp:547-578）/`OH_DDK_DestroyAshmem` |
| 状态流转 | 全局 DDK 生命周期：g_initialized=false → OH_Usb_Init 成功 → true → Release/ReleaseResource 关 fd、munmap 全部缓冲并置 false（usb_ddk_api.cpp:81-211）。进程退出前未清理的资源由 Release 统一兜底 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | offset+bufferLength 不得越界（USB_DDK_INVALID_PARAMETER，usb_ddk_api.cpp:448-453）；同地址重复 CreateDeviceMemMap 时关闭旧 fd（usb_ddk_api.cpp:528-531）；ControlTransfer 读回长度以 transferredLength 为准并 memcpy_s 校验（usb_ddk_api.cpp:635-648） |
| 并发约束 | g_fdMap/g_bufferSizeMap 持 g_fdMapMutex（unique_lock）；g_ddk 读用 shared_lock；Init/Release 由 g_lifecycleMutex 串行（usb_ddk_api.cpp:51-53） |
| 安全约束 | 权限 ohos.permission.ACCESS_DDK_USB 在 HDI 侧按调用进程 token 校验（头文件 @permission 标注，usb_ddk_api.h:51 等）；模拟器构建统一返回 DDK_EMULATOR_NOT_SUPPORT（usb_ddk_api.cpp:83） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | 依赖先 OH_Usb_Init 成功（否则 USB_DDK_INVALID_OPERATION）；deviceId 使用 DDK 编码 (busNum\<\<32)+devAddr |
| 下游影响 | usbd HDI（V1_1 传输 / V1_2 ControlTransfer、GetDevices 等，usb_ddk_api.cpp:614-671 经 CastFrom 升版） |
| 平级关联 | Base DDK 的 Ashmem 供 UsbSerial/SCSI 等场景复用（bundle.json syscap SystemCapability.Driver.DDK.Extension） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | 进程级全局资源表（g_ddk/g_fdMap/g_bufferSizeMap，usb_ddk_api.cpp:37-56）、`UsbDdkConfigDescriptor` 解析器（frameworks/ddk/usb/usb_config_desc_parser.h） |
| 核心方法 | `OH_Usb_Init/OH_Usb_Release/OH_Usb_ReleaseResource`、`OH_Usb_ClaimInterface/ReleaseInterface`、`OH_Usb_SendPipeRequest`、`OH_Usb_CreateDeviceMemMap/DestroyDeviceMemMap`、`OH_DDK_CreateAshmem/DestroyAshmem`（interfaces/ddk/usb/usb_ddk_api.h、interfaces/ddk/base/ddk_api.h） |
