// knewcode.h5 v1.0.0
// (c) 2015, 2022 shanghai mingye, Inc.
// 86my.cn

// 引入
import {
	CKcBase,
	CKcUtil,
	CKcUtilEx,
	CKcAjax,
	CKcSSE,
	CKcWS,
	CKcSrvTransfer
} from "./knewcode.js";

// 引出
export {
	CKcIDB,
	CKcGPS,
	CKcRTCMediaManager,
	CKcRTCMediaProxy,
	CKcRTCManager
};

// 错误代码
const c_errCode_Offer = 90000;
const c_errCode_Answer = 90001;
const c_errCode_nonexistent = 90002;
const c_errCode_formatError = 90003;
const c_errCode_disconnected = 90004;

// 心跳间隔5分钟（单位：毫秒）
const c_heart_interval_ms = 300 * 1000;
// 2分半钟后启动心跳（单位：毫秒）
const c_heart_begin_ms = 150 * 1000;

//////////////////////////////////////////////////////////////////////// IndexedDB //////////////////////////////////////////////////////////////////
// IndexedDB类
class CKcIDB extends CKcUtil {
	// 开始
	onopen = function(e) {
		console.log({
			"IDB_onopen": e
		});
	}
	// 失败
	onfail = function(err) {
		console.error({
			"IDB_onfail": err
		});
	}
	// 错误事件
	onerr = function(err) {
		console.error({
			"IDB_onerr": err
		});
	}
	// 创建数据结构事件
	onupgrad = function(idb) {
		console.log({
			"IDB_onupgrad": idb
		});
		let objStore = idb.createObjectStore("test", {
			"keyPath": "id"
		});
		objStore.createIndex('name', 'name', {
			unique: false
		});
	}

	// 执行
	_ExecDB(obj, func) {
		return new Promise(function(resolve, reject) {
			try {
				let rq = func(resolve);
				if (CKcUtil.isNull(rq.onsuccess)) rq.onsuccess = (e) => resolve(e.target);
				rq.onerror = (e) => reject(e.target.error);
			} catch (err) {
				reject(err);
			}
		});
	}
	_Query(obj, func) {
		let own = this;
		return this._ExecDB(obj, function(resolve) {
			return func(own._idb.transaction([obj], "readonly").objectStore(obj), resolve);
		});
	}
	_Exec(obj, func) {
		let own = this;
		return this._ExecDB(obj, function(resolve) {
			return func(own._idb.transaction([obj], "readwrite").objectStore(obj));
		});
	}

	// 增、改、删
	Insert(obj, data) {
		return this._Exec(obj, function(qry) {
			return qry.add(data);
		});
	}
	Update(obj, data) {
		return this._Exec(obj, function(qry) {
			return qry.put(data);
		});
	}
	DeleteKey(obj, key) {
		return this._Exec(obj, function(qry) {
			return qry.delete(key);
		});
	}
	// 查
	SelectAll(obj) {
		return this._Query(obj, function(qry, resolve) {
			return qry.getAll();
		});
	}
	SelectKey(obj, key) {
		return this._Query(obj, function(qry, resolve) {
			return qry.get(key);
		});
	}
	SelectIndex(obj, index, key) {
		return this._Query(obj, function(qry, resolve) {
			return qry.index(index).getAll(key);
		});
	}
	// 游标
	_SelectCursor(obj, count, func, fQry) {
		let iSort = 0;
		return this._Query(obj, function(qry, resolve) {
			let rq = fQry(qry);
			rq.onsuccess = function(e) {
				var cursor = e.target.result;
				if (cursor && (CKcUtil.isNull(count) || count-- > 0)) {
					func(cursor.value, ++iSort);
					cursor.continue();
				} else {
					//console.log('没有更多数据了！');
					resolve(e.target);
				}
			};
			return rq;
		});
	}
	SelectAllCursor(obj, func) {
		return this._SelectCursor(obj, null, func, qry => qry.openCursor());
	}
	SelectAllCursorDesc(obj, func) {
		return this._SelectCursor(obj, null, func, qry => qry.openCursor(undefined, "prev"));
	}
	SelectAllCursorCount(obj, count, func) {
		return this._SelectCursor(obj, count, func, qry => qry.openCursor());
	}
	SelectAllCursorDescCount(obj, count, func) {
		return this._SelectCursor(obj, count, func, qry => qry.openCursor(undefined, "prev"));
	}
	SelectIndexCursor(obj, index, key, desc, func) {
		return this._SelectCursor(obj, null, func, qry => qry.index(index).openCursor(key, desc ? "prev" : "next"));
	}
	SelectIndexCursorCount(obj, index, key, desc, count, func) {
		return this._SelectCursor(obj, count, func, qry => qry.index(index).openCursor(key, desc ? "prev" :
			"next"));
	}

	// 构造函数
	constructor(name, ver) {
		super();
		let own = this;

		// 数据库请求
		const oIndexedDB = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
		window.IDBTransaction = window.IDBTransaction || window.webkitIDBTransaction || window.msIDBTransaction || {
			READ_WRITE: "readwrite"
		};
		window.IDBKeyRange = window.IDBKeyRange || window.webkitIDBKeyRange || window.msIDBKeyRange;
		if (!oIndexedDB || !window.IDBTransaction || !window.IDBKeyRange)
			throw new Error("The browser doesn't support IndexedDB");
		this._reqdb = oIndexedDB.open(name, ver);
		this._reqdb.onerror = e => own.onfail(e);
		this._reqdb.onblocked = e => own.onfail(e);
		this._reqdb.onsuccess = function(e) {
			own._idb = e.target.result;
			own.onopen(e);
			own._reqdb.onerror = e => own.onerr(e);
			own._reqdb.onblocked = e => own.onerr(e);
		};
		// 创建数据结构
		this._reqdb.onupgradeneeded = function(e) {
			own._idb = e.target.result;
			own.onupgrad(own._idb);
		};
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

//////////////////////////////////////////////////////////////////////// Geolocation //////////////////////////////////////////////////////////////////
// 地理定位类
class CKcGPS extends CKcUtil {
	// 构造函数
	constructor(name, ver) {
		super();
		let own = this;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

//////////////////////////////////////////////////////////////////////// webrtc - media //////////////////////////////////////////////////////////////////
// webrtc媒体流类
class CKcRTCMedia extends CKcUtil {
	// 媒体流
	stream = null;
	// 获取流的情况
	hasAudio = false;
	hasVideo = false;
	hasScreen = false;

	// 错误事件
	onerr = function(err) {
		console.error("MEDIA_onerr", err);
	}
	// 流中断事件
	onbreak = function(strm) {
		console.log("MEDIA_onbreak", strm);
	}
	// 流开始事件
	onbegin = function(strm, sender) {
		console.log("MEDIA_onbegin", strm, sender);
	}
	// 流结束事件
	onend = function(strm) {
		console.log("MEDIA_onend", strm);
	}

	// 只获取音频流
	_GetStreamAudio(cfg) {
		console.log(cfg);
		this.hasAudio = this.hasVideo = this.hasScreen = false;
		this.stream = null;
		let own = this;
		if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
			navigator.mediaDevices.getUserMedia({
					video: false,
					audio: true
				})
				.then(function(strm) {
					own.hasAudio = true;
					own.stream = strm;
					// 中断事件
					own.stream.oninactive = function() {
						own.onbreak(own.stream);
						own.hasAudio = false;
						own.stream = null;
					};
					own.isLoading = false;
					own.onbegin(own.stream, own);
				})
				.catch(function(err) {
					own.isLoading = false;
					own.onerr({
						errCode: 3,
						errMsg: err.name + " - " + err.message
					});
				});
		} else {
			this.isLoading = false;
			this.onerr({
				errCode: 2,
				errMsg: "not exists navigator.mediaDevices.getUserMedia"
			});
		}
	}
	// 获取视频流
	async _GetStreamVideo(cfg) {
		console.log(cfg);
		this.hasAudio = this.hasVideo = this.hasScreen = false;
		let needAudio = CKcUtil.hasOwnProp(cfg, "audio") ? cfg.audio : true;
		this.stream = null;
		let own = this;
		if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
			if (null == this.stream)
				try {
					this.stream = await navigator.mediaDevices.getUserMedia({
						video: true,
						audio: needAudio,
					});
					this.hasAudio = needAudio;
					this.hasVideo = true;
				} catch (err) {
					console.warn(err);
				}
			if (null == this.stream)
				try {
					this.stream = await navigator.mediaDevices.getUserMedia({
						video: true,
						audio: false
					});
					this.hasVideo = true;
				} catch (err) {
					console.warn(err);
				}
			if (null == this.stream)
				try {
					this.stream = await navigator.mediaDevices.getUserMedia({
						video: false,
						audio: needAudio,
					});
					this.hasAudio = needAudio;
				} catch (err) {
					console.warn(err);
				}
			if (null != this.stream) {
				// 中断事件
				this.stream.oninactive = function() {
					own.onbreak(own.stream);
					own.hasAudio = own.hasVideo = false;
					own.stream = null;
				};
				this.isLoading = false;
				this.onbegin(this.stream, this);
			} else {
				this.isLoading = false;
				this.onerr({
					errCode: 5,
					errMsg: "Can't Get MediaStream"
				});
			}
			//console.log(this.hasAudio + ":" + this.hasVideo);
		} else {
			this.isLoading = false;
			this.onerr({
				errCode: 4,
				errMsg: "not exists navigator.mediaDevices.getUserMedia"
			});
		}
	}
	// 共享桌面
	_GetStreamScreen(cfg) {
		console.log(cfg);
		this.hasAudio = this.hasVideo = this.hasScreen = false;
		let needAudio = CKcUtil.hasOwnProp(cfg, "audio") ? cfg.audio : true;
		let typeCursor = CKcUtil.hasOwnProp(cfg, "cursor") ? cfg.cursor : "always";
		this.stream = null;
		let own = this;
		if (navigator.mediaDevices && navigator.mediaDevices.getDisplayMedia) {
			navigator.mediaDevices.getDisplayMedia({
					video: {
						cursor: typeCursor
					},
					audio: needAudio
				})
				.then(function(strm) {
					own.hasScreen = true;
					strm.getTracks().forEach(function(track) {
						if ("audio" === track.kind) own.hasAudio = true;
						//console.log(track.kind);
					});
					own.stream = strm;
					// 中断事件
					own.stream.oninactive = function() {
						own.onbreak(own.stream);
						own.hasAudio = own.hasScreen = false;
						own.stream = null;
					};
					own.isLoading = false;
					own.onbegin(own.stream, own);
				})
				.catch(function(err) {
					own.isLoading = false;
					own.onerr({
						errCode: 7,
						errMsg: err.name + " - " + err.message
					});
				});
		} else {
			this.isLoading = false;
			this.onerr({
				errCode: 6,
				errMsg: "not exists navigator.mediaDevices.getDisplayMedia"
			});
		}
	}
	// 开始获取媒体流。
	_Begin() {
		this.Stop();
		this.isLoading = true;
		try {
			if ("audio" === this.parm.type) this._GetStreamAudio(this.parm);
			else if ("video" === this.parm.type) this._GetStreamVideo(this.parm);
			else if ("screen" === this.parm.type) this._GetStreamScreen(this.parm);
			else throw new Error("The Media Type is Error");
		} catch (err) {
			this.onerr({
				errCode: 8,
				errMsg: err
			});
		}
	}
	Open() {
		let own = this;
		return new Promise((resolve, reject) => {
			try {
				own.onerr = err => {
					own.isOpen = false;
					reject(err);
				}
				own.onbegin = (strm, md) => {
					own.isOpen = true;
					resolve(md);
				}
				own._Begin();
			} catch (err) {
				own.isOpen = false;
				reject(err);
			}
		});
	}
	// 停止媒体流。
	Stop() {
		try {
			if (null != this.stream) {
				/*
				this.stream.getTracks().forEach(function(track) {
					track.stop();
				});
				*/
				for (let track of this.stream.getTracks())
					track.stop();
				this.onend(this.stream);
			}
		} catch (err) {
			this.onerr({
				errCode: 9,
				errMsg: err
			});
		}
		this.stream = null;
		this.hasAudio = this.hasVideo = this.hasScreen = this.isLoading = this.isOpen = false;
	}

	// 构造函数
	constructor(name, parm) {
		super();
		let own = this;

		// 参数
		this.name = name;
		this.parm = parm;

		// 初始状态
		this.isLoading = this.isOpen = false;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.Stop();
	}
}

// webrtc媒体流管理类
class CKcRTCMediaManager extends CKcUtil {
	// 本地设备列表
	devices = [];

	// 构造函数
	constructor() {
		super();
		let own = this;

		// 麦克风
		this.MIC = new CKcRTCMedia("audio", {
			type: "audio"
		});
		this.MIC._byUse = new Set();
		// 摄像头
		this.CAM = new CKcRTCMedia("video", {
			type: "video",
			audio: false
		});
		this.CAM._byUse = new Set();

		// 本地设备列表
		if (navigator.mediaDevices && navigator.mediaDevices.enumerateDevices) {
			navigator.mediaDevices.enumerateDevices()
				.then(function(devs) {
					console.table(devs);
					devs.forEach(function(dev) {
						own.devices.push({
							"kind": dev.kind,
							"label": dev.label,
							"id": dev.deviceId,
							"group": dev.groupId
						});
					});
				})
				.catch(function(err) {
					own.onerr({
						errCode: 1,
						errMsg: err.name + " - " + err.message
					});
				});
		} else throw new Error("not exists navigator.mediaDevices");
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.MIC._byUse.clear();
		this.MIC.destructor();
		delete this.MIC;
		this.CAM._byUse.clear();
		this.CAM.destructor();
		delete this.CAM;
	}
}
// webrtc媒体流代理类
class CKcRTCMediaProxy extends CKcUtil {
	// 构造函数
	constructor(manager, flag) {
		super();
		let own = this;
		this._manager = manager;
		this._flag = flag;

		// 共享桌面
		this.SCR = new CKcRTCMedia("screen", {
			type: "screen",
			audio: false
		});

		// 状态值
		this.isOpenMic = this.isOpenCamera = this.isOpenScreen = false;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.StopAll();
		this.SCR.destructor();
		delete this.SCR;
	}

	// 开关麦克风
	OpenMic() {
		let own = this;
		return new Promise((resolve, reject) => {
			if (own._manager.MIC.isOpen) {
				own.isOpenMic = true;
				own._manager.MIC._byUse.add(own._flag);
				resolve(own._manager.MIC);
			} else own._manager.MIC.Open().then(md => {
				own.isOpenMic = md.isOpen;
				if (own.isOpenMic) {
					md._byUse.add(own._flag);
					resolve(md);
				} else {
					own.StopMic();
					reject("Failed to open the microphone");
				}
			}).catch(err => {
				own.StopMic();
				reject(err);
			});
		});
	}
	StopMic() {
		this.isOpenMic = false;
		this._manager.MIC._byUse.delete(this._flag);
		if (this._manager.MIC._byUse.size <= 0)
			this._manager.MIC.Stop();
	}
	// 开关摄像头
	OpenCam() {
		let own = this;
		return new Promise((resolve, reject) => {
			if (own._manager.CAM.isOpen) {
				own.isOpenCamera = true;
				own._manager.CAM._byUse.add(own._flag);
				resolve(own._manager.CAM);
			} else own._manager.CAM.Open().then(md => {
				own.isOpenCamera = md.isOpen;
				if (own.isOpenCamera) {
					md._byUse.add(own._flag);
					resolve(md);
				} else {
					own.StopCam();
					reject("Failed to open the camera");

				}
			}).catch(err => {
				own.StopCam();
				reject(err);
			});
		});
	}
	StopCam() {
		this.isOpenCamera = false;
		this._manager.CAM._byUse.delete(this._flag);
		if (this._manager.CAM._byUse.size <= 0)
			this._manager.CAM.Stop();
	}
	// 开关共享桌面
	OpenScr() {
		let own = this;
		return new Promise((resolve, reject) => {
			own.SCR.Open().then(md => {
				own.isOpenScreen = md.isOpen;
				if (own.isOpenScreen) resolve(md);
				else reject("Failed to open the shared desktop");
			}).catch(err => {
				own.isOpenScreen = false;
				reject(err);
			});
		});
	}
	StopScr() {
		this.isOpenScreen = false;
		this.SCR.Stop();
	}
	// 关闭所有
	StopAll() {
		if (this.isOpenMic) this.StopMic();
		if (this.isOpenCamera) this.StopCam();
		if (this.isOpenScreen) this.StopScr();
		this.isOpenMic = this.isOpenCamera = this.isOpenScreen = false;
	}
}

//////////////////////////////////////////////////////////////////////// webrtc - peer //////////////////////////////////////////////////////////////////
// webrtc点对点基类
class CKcRTCPeer extends CKcUtil {
	// 构造函数
	constructor(flag, turnCfg, peerMake) {
		super();
		let own = this;
		this.sFlag = flag;
		this._peerMake = peerMake;

		// 不能创建基类对象
		if (new.target === CKcRTCPeer) throw new Error("Can't Create CKcRTCPeer Object");

		// 创建操作类对象
		this.OP = peerMake.CreatePeerOP(this);

		// 创建点对点对象的函数
		this._CreateRTCPeer = function() {
			// 判断是否支持webrtc
			let TPeerConnection = window.RTCPeerConnection || window.mozRTCPeerConnection || window.webkitRTCPeerConnection;
			if (!TPeerConnection)
				throw new Error("The browser does not support webrtc");
			// 创建点对点对象
			if (this._peer) this._peer.close();
			this._peer = new TPeerConnection(turnCfg);
			this.OP.SetupPeer(this._peer);
			return this._peer;
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.OP.destructor();
		delete this.OP;
		this._peerMake.destructor();
		delete this._peerMake;
	}
}
// webrtc点对点连接类
class CKcRTCPeerConn extends CKcRTCPeer {
	// 连接已打开事件
	onopened = function(e) {
		console.log("PEER_onopened", e, this);
	}
	// 连接失败事件
	onconnfail = function(e) {
		console.log("PEER_onconnfail", e, this);
	}

	// 构造函数
	constructor(flag, turnCfg, peerMake) {
		super(flag, turnCfg, peerMake);
		let own = this;

		// 不能创建基类对象
		if (new.target === CKcRTCPeerConn) throw new Error("Can't Create CKcRTCPeerConn Object");

		// 信令命令对象
		this._signalCmd = peerMake.CreateSignalCmd();
		this._signalCmd.onerr = e => own.onconnfail(e);

		// 创建点对点对象的函数
		this._CreateRTCPeerConn = function() {
			let own = this;
			this._CreateRTCPeer();
			// 点对点链接的nat公网信息事件
			this._peer.onicecandidate = e => {
				if (e.candidate) {
					let signal = {
						type: "ice",
						candidate: e.candidate
					};
					own._signalCmd.SendSignal(signal);
				}
			};
			return this._peer;
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this._signalCmd.destructor();
		delete this._signalCmd;
	}
}
// webrtc点对点连接请求类
class CKcRTCPeerOffer extends CKcRTCPeerConn {
	// 发送提议信令
	Start() {
		let own = this;
		try {
			this._CreateRTCPeerConn().createOffer()
				.then(offer => {
					own._peer.setLocalDescription(offer);
					own._signalCmd.SendSignal(offer);
				})
				.catch(err => own.onconnfail({
					errCode: 1,
					errMsg: err.name + " - " + err.message
				}));
		} catch (err) {
			own.onconnfail({
				errCode: 1,
				errMsg: err
			});
		}
	}

	// 构造函数
	constructor(flag, turnCfg, peerMake) {
		super(flag, turnCfg, peerMake);
		let own = this;

		// 接收信令事件
		this._signalCmd.onsignal = async function(signal) {
			try {
				if ("answer" === signal.type)
					await own._peer.setRemoteDescription(new RTCSessionDescription(signal));
				else if ("ice" === signal.type)
					await own._peer.addIceCandidate(signal.candidate);
			} catch (err) {
				own.onconnfail({
					errCode: 1,
					errMsg: err
				});
			}
			console.log("PEEROFFER_onsignal", own._signalCmd, signal);
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}
// webrtc点对点连接应答类
class CKcRTCPeerAnswer extends CKcRTCPeerConn {
	// 构造函数
	constructor(flag, turnCfg, peerMake) {
		super(flag, turnCfg, peerMake);
		let own = this;

		// 接收信令事件
		this._signalCmd.onsignal = async function(signal) {
			try {
				if ("offer" === signal.type) {
					await own._CreateRTCPeerConn().setRemoteDescription(new RTCSessionDescription(signal));
					let answer = await own._peer.createAnswer();
					await own._peer.setLocalDescription(answer);
					own._signalCmd.SendSignal(answer);
				} else if ("ice" === signal.type)
					await own._peer.addIceCandidate(signal.candidate);
			} catch (err) {
				own.onconnfail({
					errCode: 1,
					errMsg: err
				});
			}
			console.log("PEERANSWER_onsignal", own._signalCmd, signal);
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

//////////////////////////////////////////////////////////////////////// webrtc - signal //////////////////////////////////////////////////////////////////
// webrtc信令收发类（通过SSE）
class CKcRTCSignalSSE_RS extends CKcUtil {
	// 首次接收事件
	onfirstrecv = function(jsn) {
		console.log("SIGNAL_onnewrecv", this._p2pUserSignal._objSSE.sid, jsn);
	}

	// 发送信令的函数
	SendSignal(flag, toID, signal) {
		return this._p2pUserSignal.SendData(this._signalUri, {
			"to": toID,
			"data": {
				"eventSignal": this.eventSignalName(flag, this._p2pUserSignal._objSSE.sid),
				"flag": flag,
				"signal": signal
			}
		}, this._p2pSigFName);
	}

	// 信号分发事件名
	eventSignalName(flag, toID) {
		return this._p2pSigFName + "_" + flag + "__" + toID;
	}

	// 构造函数
	constructor(objSSE, signalUri) {
		super();
		let own = this;

		// 自己的sse编号
		this.sid = objSSE.sid;
		// 发送信令的url地址
		this._signalUri = signalUri;

		// 服务器信令中转类实例
		this._p2pSigFName = "p2pSignal";
		this._p2pUserSignal = new CKcSrvTransfer(objSSE, this._p2pSigFName);
		this._p2pUserSignal.onrecv = jsn => {
			// 触发事件
			if (CKcUtil.hasOwnProp(jsn, "eventSignal") && own.dispatchEvent(jsn.eventSignal, jsn) === 0)
				own.onfirstrecv(jsn);
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this._p2pUserSignal.destructor();
		delete this._p2pUserSignal;
	}
}
// webrtc信令命令类（通过SSE）
class CKcRTCSignalSSE extends CKcUtil {
	// 信令事件
	onsignal = function(signal) {
		console.log("SIGNAL_onsignal", this.flag, this.toID, this._signalSend.sid, signal, this);
	}
	// 错误事件
	onerr = function(err) {
		console.error("SIGNAL_onerr", this.flag, this.toID, this._signalSend.sid, err, this);
	}

	// 发送信令的函数
	SendSignal(signal) {
		let own = this;
		//console.log("SIGNAL_SendSignal", this.toID, this._signalSend.sid, signal);
		this._signalSend.SendSignal(this.flag, this.toID, signal).catch(err => own.onerr({
			errCode: 1,
			errMsg: err,
			signal: signal
		}));
	}

	// 构造函数
	constructor(signalSend, flag, toID) {
		super();
		let own = this;

		// 唯一标志
		this.flag = flag;
		// 对方的编号
		this.toID = toID;
		// 信令收发类对象
		this._signalSend = signalSend;

		// 信令事件
		this._eventSignalName = this._signalSend.eventSignalName(flag, toID);
		this._eventSignalFN = data => own.onsignal(data.signal);
		this._signalSend.addEventListener(this._eventSignalName, this._eventSignalFN);
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this._signalSend.removeEventListener(this._eventSignalName, this._eventSignalFN);
	}
}

// webrtc信令命令类（通过点对点）
class CKcRTCSignalPeer extends CKcUtil {
	// 信令事件
	onsignal = function(signal) {
		console.log("SIGNAL_onsignal", this.sType, signal, this);
	}
	// 错误事件
	onerr = function(err) {
		console.error("SIGNAL_onerr", this.sType, err, this);
	}

	// 构造函数
	constructor(DataOP, type, isSend) {
		super();
		let own = this;

		// 信令收发类对象
		this._DataOP = DataOP;

		// 发送信令的函数
		this.SendSignal = function(signal) {
			let own = this;
			let eventSendName = this._DataOP.getEventNameSignal(type) + (isSend ? "_R" : "_S");
			return this._DataOP.SendSignal(eventSendName, signal);
		}

		// 信令事件
		this._eventRecvName = this._DataOP.getEventNameSignal(type) + (isSend ? "_S" : "_R");
		this._eventSignalFN = signal => own.onsignal(signal);
		this._DataOP.addEventListener(this._eventRecvName, this._eventSignalFN);
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this._DataOP.removeEventListener(this._eventRecvName, this._eventSignalFN);
	}
}

//////////////////////////////////////////////////////////////////////// webrtc - operate //////////////////////////////////////////////////////////////////
// webrtc点对点操作类
class CKcRTCPeerOP extends CKcUtil {
	// 断线事件
	ondiscon = function(err) {
		console.log("PEER_ondiscon", err, this);
	}
	// 错误
	onerr = function(err) {
		console.log("PEER_onerr", err, this);
	}

	// 构造函数
	constructor(peer) {
		super();
		let own = this;
		this.PR = peer;

		// 不能创建基类对象
		if (new.target === CKcRTCPeerOP) throw new Error("Can't Create CKcRTCPeerOP Object");
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.Close();
	}

	// 设置点对点对象(事件等)
	SetupPeer(pr) {
		let own = this;
		// 点对点链接的断线事件
		pr.oniceconnectionstatechange = e => {
			console.log("oniceconnectionstatechange", e, own);
			if (e.currentTarget.iceConnectionState === "disconnected") {
				// 断线5秒后，关闭连接
				setTimeout(function() {
					if (!own.IsConnect()) {
						// 关闭连接
						own.Close();
						// 触发事件
						own.ondiscon({
							errCode: 1,
							errMsg: "断线",
							event: e
						});
					}
				}, 5555);
				//pr.restartIce();
			}
		};
	}

	// 是否链接
	IsConnect() {
		return CKcUtil.hasOwnProp(this.PR, "_peer") && this.PR._peer &&
			CKcUtil.hasArrayVal([this.PR._peer.connectionState, this.PR._peer.iceConnectionState], "connected")
		//&& (this.PR._peer.connectionState === "connected" || this.PR._peer.iceConnectionState === "connected")
		;
	}
	CheckConnect() {
		let isConn = this.IsConnect();
		if (!isConn) this.onerr({
			errCode: c_errCode_disconnected,
			errMsg: "disconnected"
		});
		return isConn;
	}

	// 关闭链接
	Close() {
		if (this._CloseRes) this._CloseRes();
		if (this.IsConnect())
			this.PR._peer.close();
		delete this.PR._peer;
	}
}

// webrtc点对点媒体流操作类
class CKcRTCPeerMediaOP extends CKcRTCPeerOP {
	// 构造函数
	constructor(peer) {
		super(peer);
		let own = this;

		// 不能创建基类对象
		if (new.target === CKcRTCPeerMediaOP) throw new Error("Can't Create CKcRTCPeerMediaOP Object");
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}
// webrtc点对点媒体流操作类（发送）
class CKcRTCPeerMediaOPSend extends CKcRTCPeerMediaOP {
	// 构造函数
	constructor(DataOP, peer, Media) {
		super(peer);
		let own = this;

		this.MD = Media;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}

	// 释放资源
	_CloseRes() {
		this._ClearMedia();
	}

	// 设置点对点对象(事件等)
	SetupPeer(pr) {
		super.SetupPeer(pr);
		// 添加媒体流
		this.MD.stream.getTracks().forEach(track => {
			pr.addTrack(track, this.MD.stream);
			console.log("addTrack", this.PR, track);
		});
	}

	// 移除本地媒体流
	_RemoveMedia(track) {
		let own = this;
		if (this.PR._peer)
			this.PR._peer.getSenders().forEach(sender => {
				if (track === sender.track)
					own.PR._peer.removeTrack(sender);
			});
	}
	// 清除本地媒体流
	_ClearMedia() {
		let own = this;
		if (this.PR._peer)
			this.PR._peer.getSenders().forEach(sender => own.PR._peer.removeTrack(sender));
	}
}
class CKcRTCPeerMediaOPSendMake extends CKcUtil {
	// 构造函数
	constructor(DataOP, Media) {
		super();
		let own = this;

		// 创建信令收发类对象
		this.CreateSignalCmd = function() {
			return new CKcRTCSignalPeer(DataOP, Media.name, true);
		}

		// 创建点对点媒体流管理类对象
		this.CreatePeerOP = function(peer) {
			return new CKcRTCPeerMediaOPSend(DataOP, peer, Media);
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}
// webrtc点对点媒体流操作类（接收）
class CKcRTCPeerMediaOPRecv extends CKcRTCPeerMediaOP {
	// 对方媒体流事件
	OnTrack = function(e, peer) {
		console.log("PEER_OnTrack", e, peer);
	}

	// 构造函数
	constructor(DataOP, peer, type) {
		super(peer);
		let own = this;
		this.DataOP = DataOP;
		this._type = type;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}

	// 释放资源
	_CloseRes() {
		// 实时语音流事件
		if ("audio" === this._type)
			this.DataOP.PMM._StopAudio();
		// 实时视频流事件
		else if ("video" === this._type)
			this.DataOP.PMM._StopVideo();
		// 实时桌面事件
		else if ("screen" === this._type)
			this.DataOP.PMM._StopScreen();
	}

	// 设置点对点对象(事件等)
	SetupPeer(pr) {
		let own = this;
		super.SetupPeer(pr);
		// 媒体流协商事件
		pr.onnegotiationneeded = e => {
			console.log("onnegotiationneeded", own.PR, e);
		};
		// 获取对方媒体流
		pr.ontrack = e => {
			console.log("ontrack", own.PR, e);
			// 实时语音流事件
			if ("audio" === own._type)
				own.DataOP.PMM.OnTrackAudio(e, own.PR);
			// 实时视频流事件
			else if ("video" === own._type)
				own.DataOP.PMM.OnTrackVideo(e, own.PR);
			// 实时桌面事件
			else if ("screen" === own._type)
				own.DataOP.PMM.OnTrackScreen(e, own.PR);
		};
	}
}
class CKcRTCPeerMediaOPRecvMake extends CKcUtil {
	// 构造函数
	constructor(DataOP, type) {
		super();
		let own = this;

		// 创建信令收发类对象
		this.CreateSignalCmd = function() {
			return new CKcRTCSignalPeer(DataOP, type, false);
		}

		// 创建点对点媒体流管理类对象
		this.CreatePeerOP = function(peer) {
			return new CKcRTCPeerMediaOPRecv(DataOP, peer, type);
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

// webrtc点对点媒体流管理类
class CKcRTCPeerMediaManager extends CKcUtil {
	// 对方的实时语音流事件
	OnTrackAudio = function(e, peer) {
		console.log("PEER_OnTrackAudio", e, peer);
		this._OpenAudio(e, peer);
	}
	// 对方的实时视频流事件
	OnTrackVideo = function(e, peer) {
		console.log("PEER_OnTrackVideo", e, peer);
		this._video = this._OpenVideo("video", e, peer);
	}
	// 对方的共享桌面视频流事件
	OnTrackScreen = function(e, peer) {
		console.log("PEER_OnTrackScreen", e, peer);
		this._screen = this._OpenVideo("screen", e, peer);
	}
	// 打开实时视频控件事件
	OnOpenVideoCtrl = function(ctrl, e, peer) {
		console.log("PEER_OnOpenVideoCtrl", ctrl, e, peer);
	}
	// 打开共享桌面控件事件
	OnOpenScreenCtrl = function(ctrl, e, peer) {
		console.log("PEER_OnOpenScreenCtrl", ctrl, e, peer);
	}
	// 关闭实时视频控件事件
	OnCloseVideoCtrl = function(ctrl, e, peer, ev) {
		console.log("PEER_OnCloseVideoCtrl", ctrl, e, peer);
	}
	// 关闭共享桌面控件事件
	OnCloseScreenCtrl = function(ctrl, e, peer, ev) {
		console.log("PEER_OnCloseScreenCtrl", ctrl, e, peer);
	}

	// 构造函数
	constructor(DataOP, turnCfg) {
		super();
		let own = this;
		this.DataOP = DataOP;

		// 媒体点对点对象列表
		this._peerMediaSends = new Map();
		this._peerMediaRecvs = new Map();

		// 点对点媒体发送请求连接函数
		this.RePeerOffer = function(name) {
			if (this._peerMediaSends.has(name)) {
				let peerOffer = this._peerMediaSends.get(name);
				// 开始请求连接
				peerOffer.Start();
			}
		}
		this.PeerOffer = function(md) {
			this.PeerOfferClose(md.name);
			// 创建请求点对点对象
			let peerMake = new CKcRTCPeerMediaOPSendMake(DataOP, md);
			let peerOffer = new CKcRTCPeerOffer(DataOP.PR.sFlag + "/" + md.name, turnCfg, peerMake);
			// 保存点对点对象
			this._peerMediaSends.set(md.name, peerOffer);
			// 开始请求连接
			this.RePeerOffer(md.name);
		}
		// 关闭请求的连接
		this.PeerOfferClose = function(name) {
			if (this._peerMediaSends.has(name)) {
				let peerOffer = this._peerMediaSends.get(name);
				if (peerOffer.OP.IsConnect())
					DataOP.SendCmd("kc_peer_closeAnswer", {
						"type": name
					});
				peerOffer.destructor();
				this._peerMediaSends.delete(name);
			}
		}

		// 点对点媒体接收准备连接函数
		this.PeerAnswer = function(name) {
			this.PeerAnswerClose(name);
			// 创建应答点对点对象
			let peerMake = new CKcRTCPeerMediaOPRecvMake(DataOP, name);
			let peerAnswer = new CKcRTCPeerAnswer(DataOP.PR.sFlag + "/" + name, turnCfg, peerMake);
			// 保存点对点对象
			this._peerMediaRecvs.set(name, peerAnswer);
			// 通知对方来请求
			DataOP.SendCmd("kc_peer_canMediaOffer", name);
		}
		// 关闭应答的连接
		this.PeerAnswerClose = function(name) {
			if (this._peerMediaRecvs.has(name)) {
				let peerAnswer = this._peerMediaRecvs.get(name);
				if (peerAnswer.OP.IsConnect())
					DataOP.SendCmd("kc_peer_closeOffer", {
						"type": name
					});
				peerAnswer.destructor();
				this._peerMediaRecvs.delete(name);
			}
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		// 关闭实时语音
		this.StopAudio();
		// 关闭实时视频
		this.StopVideo();
		// 清除点对点对象
		this._peerMediaSends.forEach((peer, key, mp) => peer.destructor());
		this._peerMediaRecvs.forEach((peer, key, mp) => peer.destructor());
		this._peerMediaSends.clear();
		this._peerMediaRecvs.clear();
	}

	// 实时语音
	_OpenAudio(e, peer) {
		this._StopAudio();
		this._audio = new Audio();
		this._audio.volume = 1;
		this._audio.srcObject = e.streams[0];
		this._audio.play();
	}
	_StopAudio() {
		if (this._audio) {
			this._audio.pause();
			delete this._audio;
		}
	}
	StopAudio() {
		this.PeerAnswerClose("audio");
		this._StopAudio();
	}
	// 实时视频
	_OpenVideo(type, e, peer) {
		let own = this;
		// 先关闭视频
		if ("screen" === type) this._StopScreen();
		else this._StopVideo();
		const isPhoneBrw = CKcUtil.isPhoneBrower();
		// 打开视频控件
		let div = CKcUtil.OpenVideoCtrl(
			// 视频播放事件
			(video, div) => {
				// 重置样式
				div.style.width = "screen" === type ? (isPhoneBrw ? '1366px' : '72%') : (isPhoneBrw ? '100%' : '480px');
				//div.style.height = "screen" === type ? (isPhoneBrw ? '82%' : '90%') : (isPhoneBrw ? '82%' : '480px');
				if ("screen" === type && isPhoneBrw) {
					div.style.transform = "rotate(270deg)";
					div.style.top = '300px';
					div.style.left = '-260px';
				} else div.style.top = div.style.left = '0px';
				div.style.zIndex = "screen" === type ? 99 : 100;
				// 播放视频流
				video.srcObject = e.streams[0];
			},
			// 关闭控件事件
			(div, ev) => {
				if ("screen" === type) own.OnCloseScreenCtrl(div, e, peer, ev);
				else own.OnCloseVideoCtrl(div, e, peer, ev);
			});
		// 打开视频控件事件
		if ("screen" === type) this.OnOpenScreenCtrl(div, e, peer);
		else this.OnOpenVideoCtrl(div, e, peer);
		return div;
	}
	_StopVideo() {
		if (this._video) {
			this._video._video.pause();
			document.body.removeChild(this._video);
			delete this._video;
		}
	}
	StopVideo() {
		this.PeerAnswerClose("video");
		this._StopVideo();
	}
	_StopScreen() {
		if (this._screen) {
			this._screen._video.pause();
			document.body.removeChild(this._screen);
			delete this._screen;
		}
	}
	StopScreen() {
		this.PeerAnswerClose("screen");
		this._StopScreen();
	}
}

// webrtc点对点数据通道操作类
class CKcRTCPeerDataOP extends CKcRTCPeerOP {
	// 数据通道消息事件
	onchanmsg = function(msg) {
		console.log("PEER_onchanmsg", msg);
	}
	// 信令通道信令消息事件
	onchansignal = function(event, signal) {
		console.log("PEER_onchansignal", event, signal);
	}
	// 信令通道命令消息事件
	onchancmd = function(cmd, data) {
		console.log("PEER_onchancmd", cmd, data);
	}

	// 构造函数
	constructor(peer, turnCfg) {
		super(peer);
		let own = this;

		// 不能创建基类对象
		if (new.target === CKcRTCPeerDataOP) throw new Error("Can't Create CKcRTCPeerDataOP Object");

		// 媒体点对点管理类
		this.PMM = new CKcRTCPeerMediaManager(this, turnCfg);

		// 数据通道列表
		this._dataChans = new Map();

		// 发送失败的次数
		this._sendFailCount = 0;

		// 心跳时间
		this.dtActive = Date.now();
		// 心跳定时器
		this._heartCheck = {
			timeout: c_heart_interval_ms,
			isFirst: true,
			timeoutObj: null,
			reset: function() {
				clearTimeout(this.timeoutObj);
				this.isFirst = true;
				this.start();
			},
			start: function() {
				this.timeoutObj = setTimeout(function() {
					own._heartCheck.isFirst = false;
					own._heartCheck.start();
					own.SendCmd("kc_peer_heartActive");
				}, this.isFirst ? 6688 : this.timeout);
			},
			stop: function() {
				clearTimeout(this.timeoutObj);
			}
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		// 释放媒体管理
		this.PMM.destructor();
		delete this.PMM;
		// 关闭数据通道
		this._dataChans.forEach((chan, key, mp) => chan.close());
		this._dataChans.clear();
	}

	// 释放资源
	_CloseRes() {
		this._heartCheck.stop();
	}

	// 发送
	_SendData(chan, json) {
		let sResult = null;
		try {
			if (chan) {
				chan.send(JSON.stringify(json));
				this._sendFailCount = 0;
			}
		} catch (err) {
			sResult = err;
			++this._sendFailCount;
			console.error("Send Error", err);
		}
		return sResult;
	}
	// 发送信令
	SendSignal(event, signal) {
		if (this.CheckConnect()) {
			let sResult = this._SendData(this._dataChans.get("_signal"), {
				"type": "signal",
				"event": event,
				"data": signal
			});
			console.log("PEER_SendSignal: " + event, signal, sResult);
		}
	}
	// 发送命令
	SendCmd(cmd, data = {}) {
		if (this.CheckConnect()) {
			let sResult = this._SendData(this._dataChans.get("_signal"), {
				"type": "cmd",
				"cmd": cmd,
				"data": data
			});
			console.log("PEER_SendCmd: " + cmd, data, sResult);
		}
	}
	// 发送消息
	SendMsg(msg) {
		if (this.CheckConnect()) {
			let sResult = this._SendData(this._dataChans.get("_msg"), {
				"type": "msg",
				"data": msg
			});
			console.log("PEER_SendMsg", msg, sResult);
		}
	}
	// 发送文件
	SendFile(data) {
		if (this.CheckConnect())
			this._SendData(this._dataChans.get("_file"), {
				"type": "image",
				"filename": "a.jpg",
				"data": data
			});
	}
	// 数据通道消息接收
	async _RecvDataMsg(label, data) {
		let json = JSON.parse(data);
		// 信令通道
		if ("_signal" === label && CKcUtil.hasOwnProp(json, "type") && CKcUtil.hasOwnProp(json, "data")) {
			console.log("PEER_RecvDataSignal", label, json);
			// 信令消息
			if ("signal" === json.type && CKcUtil.hasOwnProp(json, "event")) {
				// 触发事件
				if (this.dispatchEvent(json.event, json.data) === 0)
					this.onchansignal(json.event, json.data);
			}
			// 命令消息
			else if ("cmd" === json.type && CKcUtil.hasOwnProp(json, "cmd")) {
				// 心跳命令
				if ("kc_peer_heartActive" === json.cmd)
					this.dtActive = Date.now();
				// 媒体点对点应答准备就绪，可以请求连接
				else if ("kc_peer_canMediaOffer" === json.cmd)
					this.PMM.RePeerOffer(json.data);
				// 媒体点对点关闭
				else if ("kc_peer_closeAnswer" === json.cmd) {
					if (this.PMM._peerMediaRecvs.has(json.data.type)) {
						let peerAnswer = this.PMM._peerMediaRecvs.get(json.data.type);
						peerAnswer.OP.Close();
					}
				} else if ("kc_peer_closeOffer" === json.cmd) {
					if (this.PMM._peerMediaSends.has(json.data.type)) {
						let peerOffer = this.PMM._peerMediaSends.get(json.data.type);
						peerOffer.OP.Close();
					}
				}
				// 其他命令
				else this.onchancmd(json.cmd, json.data);
			}
			// 格式错误
			else this.onerr({
				errCode: c_errCode_formatError,
				errMsg: "Data Format Error: " + data.substr(0, 500)
			});
		}
		// 消息通道
		else if ("_msg" === label && CKcUtil.hasOwnProp(json, "data")) {
			console.log("PEER_RecvDataMsg", label, json);
			this.onchanmsg(json.data);
		}
		// 文件通道
		else if ("_file" === label && CKcUtil.hasOwnProp(json, "data")) {
			console.log(label);
		}
		// 格式错误
		else this.onerr({
			errCode: c_errCode_formatError,
			errMsg: "Data Format Error: " + data.substr(0, 500)
		});
	}

	// 信令事件名称
	getEventNameSignal(type) {
		return "SubPeerSignal_" + type;
	}
}
// webrtc点对点数据通道操作类（针对请求连接）
class CKcRTCPeerDataOPOffer extends CKcRTCPeerDataOP {
	// 构造函数
	constructor(peer, turnCfg) {
		super(peer, turnCfg);
		let own = this;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}

	// 设置点对点对象(事件等)
	SetupPeer(pr) {
		let own = this;
		super.SetupPeer(pr);
		// 创建数据通道的函数
		let fCreateDataChan = function(label) {
			let chan = pr.createDataChannel(label);
			chan.onopen = e => {
				console.log("PEEROFFER_chan_onopen", e);
				if ("_signal" === label) {
					// 启动心跳
					own._heartCheck.reset();
					// 连接成功事件
					own.PR.onopened(e);
				}
			}
			chan.onmessage = function(e) {
				//console.log(event.data);
				own._RecvDataMsg(label, e.data);
			}
			own._dataChans.set(label, chan);
		}
		// 创建3个常用数据通道
		fCreateDataChan("_msg");
		fCreateDataChan("_file");
		fCreateDataChan("_signal");
	}
}
// webrtc点对点数据通道操作类（针对应答连接）
class CKcRTCPeerDataOPAnswer extends CKcRTCPeerDataOP {
	// 构造函数
	constructor(peer, turnCfg) {
		super(peer, turnCfg);
		let own = this;
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}

	// 设置点对点对象(事件等)
	SetupPeer(pr) {
		let own = this;
		super.SetupPeer(pr);
		// 获取数据通道
		pr.ondatachannel = eChan => {
			own._dataChans.set(eChan.channel.label, eChan.channel);
			eChan.channel.onopen = e => {
				console.log("PEERANSWER_chan_onopen", e);
				if ("_signal" === eChan.channel.label) {
					// 2分半钟后，启动心跳
					setTimeout(() => own._heartCheck.reset(), c_heart_begin_ms)
					// 连接成功事件
					own.PR.onopened(e);
				}
			}
			eChan.channel.onmessage = function(e) {
				//console.log(e.data);
				own._RecvDataMsg(eChan.channel.label, e.data);
			}
		}
	}
}
// webrtc点对点数据通道管理对象创建类
class CKcRTCPeerDataMake extends CKcUtil {
	// 构造函数
	constructor(signalRS, flag, sid, turnCfg, ClassDOP) {
		super();
		let own = this;

		// 创建信令收发类对象
		this.CreateSignalCmd = function() {
			return new CKcRTCSignalSSE(signalRS, flag, sid);
		}

		// 创建点对点媒体流管理类对象
		this.CreatePeerOP = function(peer) {
			return new ClassDOP(peer, turnCfg);
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

//////////////////////////////////////////////////////////////////////// webrtc - manager //////////////////////////////////////////////////////////////////
// webrtc管理类
class CKcRTCManager extends CKcUtil {
	// 点对点应答事件
	onpeeranswer = function(peerAnswer, i_user, from) {
		console.log("PEER_onpeeranswer", peerAnswer, i_user, from);
	}

	// 构造函数
	constructor(turnCfg, objSSE, uriSrvTrans) {
		super();
		let own = this;
		console.log("****** Start [" + CKcUtil.getKCCLNID() + " —— " + CKcUtil.getKCSSID() + " —— " + objSSE.sid + "] ******");

		// 信令收发类实例（通过SSE）
		this._signalRS = new CKcRTCSignalSSE_RS(objSSE, uriSrvTrans);
		// 对方的sse编号，首次接收消息
		this._signalRS.onfirstrecv = jsn => {
			// 创建应答点对点对象
			let peerMake = new CKcRTCPeerDataMake(this._signalRS, jsn.flag, jsn.from, turnCfg, CKcRTCPeerDataOPAnswer);
			let peerAnswer = new CKcRTCPeerAnswer(jsn.from, turnCfg, peerMake);
			// 应答事件
			peerAnswer._signalCmd.onsignal(jsn.signal);
			own.onpeeranswer(peerAnswer, jsn.i_user, jsn.from);
		}

		// 点对点连接请求函数
		this.PeerOffer = function(sid) {
			// 时间标志
			let iFlag = parseInt(Date.now() / 1000) % 1000000;
			// 创建请求点对点对象
			let peerMake = new CKcRTCPeerDataMake(this._signalRS, iFlag, sid, turnCfg, CKcRTCPeerDataOPOffer);
			let peerOffer = new CKcRTCPeerOffer(sid, turnCfg, peerMake);
			// 开始请求连接
			peerOffer.Start();
			return peerOffer;
		}
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this._signalRS.destructor();
		delete this._signalRS;
	}
}
