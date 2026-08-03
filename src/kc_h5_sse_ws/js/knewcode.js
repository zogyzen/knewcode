// knewcode v1.0.0
// (c) 2015, 2022 shanghai mingye, Inc.
// 86my.cn

// 引出
export {CKcBase, CKcUtil, CKcUtilEx, CKcAjax, CKcSSE, CKcWS, CKcSrvTransfer};

// 超时时限15分钟（单位：毫秒）
const c_timeout_ms = 900 * 1000;

// 通用基类
class CKcBase {
	// 构造函数
	constructor() {
		// 不能创建基类对象
		if (new.target === CKcBase)
			throw new Error("Can't Create CKcBase Object");
	}
	// 析构函数（需手工调用）
	destructor() {}
}
// 通用方法类
class CKcUtil extends CKcBase {
	// 判断对象是否为空
	static isNull(obj) {
		return (obj === null || obj === undefined || typeof obj == "undefined");
	}
	static nvl(obj, def) {
		return CKcUtil.isNull(obj) ? def : obj;
	}
	// 去除字符串前后的空白字符
	static trim(str) {
		return CKcUtil.isNull(str) ? "" : str.replace(/(^\s*)|(\s*$)/g, "");
	}
	// 判断字符串是否为空
	static isEmpty(str) {
		return (CKcUtil.isNull(str) || str == "");
	}
	// 判断字符串不为空白
	static isBlank(str) {
		return (CKcUtil.isEmpty(str) || CKcUtil.trim(str) == "");
	}
	// 判断变量是否为Json
	static isJson(obj) {
		return !CKcUtil.isNull(obj) && typeof(obj) === "object" && Object.prototype.toString.call(obj)
			.toLowerCase() === "[object object]" && Object.values(obj).length > 0;
	}
	// 判断Json是否为空
	static isJsonEmpty(obj) {
		if (!CKcUtil.isNull(obj))
			for (let key in obj)
				return false;
		return true;
	}
	static isJsonEmpty2(obj) {
		return Object.keys(obj).length === 0;
	}
	// 判断是否存在某属性
	static hasOwnProp(obj, prop) {
		return !CKcUtil.isNull(obj) && !CKcUtil.isBlank(prop) && Object.prototype.hasOwnProperty.call(obj, prop);
	}
	// 判断数组里是否包含某元素
	static hasArrayVal(arr, val) {
		//return arr.indexOf(val, 0) >= 0;
		return arr.includes(val);
	}
	// 按条件筛选数组数据
	static filterArray(arr, ftor) {
		return arr.filter(item => {
			return ftor(item);
		});
	}
	// 按条件查找数组数据，返回序号
	static findIndexArray(arr, ftor) {
		return arr.findIndex(item => {
			return ftor(item);
		});
	}

	// 左填充
	static paddingLeft(num, length, fill = "0") {
		return (Array(length).join(fill) + num).slice(-length);
	}

	// 判断是否数值
	static isNumber(obj) {
		//return typeof obj === 'number' && !isNaN(obj);
		return !CKcUtil.isBlank(obj) && !isNaN(Number(obj));
	}

	// 休眠
	static sleep(time) {
		return new Promise((resolve) => setTimeout(resolve, time));
	}
	// 得到超时时限
	static getTimeoutMS() {
		return c_timeout_ms;
	}

	// 获取url上的get参数
	static getQueryVariable(variable) {
		var query = window.location.search.substring(1);
		var vars = query.split("&");
		for (var i = 0; i < vars.length; i++) {
			var pair = vars[i].split("=");
			if (pair[0] == variable) {
				return pair[1];
			}
		}
		return null;
	}

	//写入cookies
	static setCookie(name, value) {
		document.cookie = name + "=" + escape(value);
	}
	static setCookieByDays(name, value, Days) {
		let exp = new Date();
		exp.setTime(exp.getTime() + Days * 24 * 60 * 60 * 1000);
		document.cookie = name + "=" + escape(value) + ";expires=" + exp.toGMTString();
	}
	//读取cookies
	static getCookie(name) {
		let reg = new RegExp("(^| )" + name + "=([^;]*)(;|$)");
		let arr = document.cookie.match(reg);
		if (arr) return unescape(arr[2]);
		return null;
	}
	//删除cookies
	static delCookie(name) {
		let exp = new Date();
		exp.setTime(exp.getTime() - 1);
		let cval = CKcUtil.getCookie(name);
		if (cval)
			document.cookie = name + "=;expires=" + exp.toGMTString();
	}

	// 本地存储
	static setLocalStorage(name, value) {
		localStorage.setItem(name, value);
	}
	static getLocalStorage(name) {
		return localStorage.getItem(name);
	}
	static delLocalStorage(name) {
		localStorage.removeItem(name);
	}
	// 会话存储
	static setSessionStorage(name, value) {
		sessionStorage.setItem(name, value);
	}
	static getSessionStorage(name) {
		return sessionStorage.getItem(name);
	}
	static delSessionStorage(name) {
		sessionStorage.removeItem(name);
	}

	// 得到设备系统类型
	static getDeviceOSType() {
		let userAgentInfo = window.navigator.userAgent;
		if (userAgentInfo.indexOf("Android") > 0)
			return "Android";
		else if (userAgentInfo.indexOf("iOS") > 0 || userAgentInfo.indexOf("iPhone") > 0 || userAgentInfo.indexOf(
				"iPad") > 0)
			return "iOS";
		else if (userAgentInfo.indexOf("Windows") > 0)
			return "Windows";
		else if (userAgentInfo.indexOf("Linux") > 0)
			return "Linux";
		else
			return "";
	}
	// 是否移动设备
	static isMobileDevice() {
		let osType = CKcUtil.getDeviceOSType();
		return osType === "Android" || osType === "iOS";
	}
	// 是否微信浏览器
	static isWeixinBrower() {
		return window.navigator.userAgent.indexOf("MicroMessenger") > 0;
	}
	// 是否竖屏（一般是手机）
	static isPortraitScreen() {
		return window.screen.width < window.screen.height;
	}
	// 通过分辨率判断是否竖屏手机浏览器
	static isPhoneBrower() {
		return window.screen.width < 768;
	}

	// 判断是否绝对地址
	static isAbsUrl(uri) {
		let sUri = uri.trim();
		let matchHttp = /^((http|https):\/\/)?(([a-z0-9]+-[a-z0-9]+|[a-z0-9]+)\.)+([a-z]+)[/?:]?.*$/;
		return matchHttp.test(sUri.toLowerCase());
	}
	// 修改uri
	static addParmAtUrl(uri, parm) {
		return uri + (uri.indexOf('?') < 0 ? "?" : "&") + parm;
	}
	static resetUrlByBOM(uri) {
		window.history.replaceState('', '', uri);
	}
	static addParmToUrlByBOM(uri, parm) {
		let sNewUrl = CKcUtil.addParmAtUrl(uri, parm);
		CKcUtil.resetUrlByBOM(sNewUrl);
		return sNewUrl;
	}
	
	// 密码复杂度检查
	static checkpassword(value){
	  return /^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[^]{8,16}$/.test(value)
	};


	// 得到文件的base64编码
	static getFileBase64(file) {
		return new Promise((resolve, reject) => {
			const reader = new FileReader();
			reader.readAsDataURL(file);
			reader.onload = () => resolve(reader.result);
			reader.onerror = error => reject(error);
		});
	}

	// 页面禁止复制
	static prohibitCopy() {
		// 禁用右键
		document.oncontextmenu = new Function("event.returnValue=false");
		// 禁用选择
		document.onselectstart = new Function("event.returnValue=false");
	}

	// 控件可移动
	static ctrlCanMove(el) {
		el.onmousedown = function(e) {
			document.onselectstart = function() {
				return false
			}
			const disx = e.pageX - el.offsetLeft
			const disy = e.pageY - el.offsetTop
			if (e.preventDefault) {
				e.preventDefault()
			} else {
				e.returnValue = false
			}
			document.onmousemove = function(e) {
				let left = e.clientX - disx
				let top = e.clientY - disy

				if (left <= 0) {
					left = 5 // 设置成5是为了不离边缘太近
				} else if (left > document.documentElement.clientWidth - el.clientWidth) {
					// document.documentElement.clientWidth 屏幕的可视宽度
					left = document.documentElement.clientWidth - el.clientWidth - 5
				}
				if (top <= 0) {
					top = 5
				} else if (top > document.documentElement.clientHeight - el.clientHeight) {
					top = document.documentElement.clientHeight - el.clientHeight - 5
					console.log(document.documentElement.clientHeight, el.clientHeight)
				}
				el.style.left = left + 'px'
				el.style.top = top + 'px'
			}
			document.onmouseup = function() {
				document.onmousemove = document.onmouseup = null
			}
		}
	}
	static ctrlCanMoveCancel(el) {
		el.onmousedown = function(e) {
			e.stopPropagation()
		}
	}

	// 获取图片分辨率
	static getImageResolution(file) {
		return new Promise((resolve, reject) => {
			let _URL = window.URL || window.webkitURL;
			let img = new Image();
			img.src = _URL.createObjectURL(file);
			img.onload = () => resolve({
				width: img.width,
				height: img.height
			});
			img.onerror = error => reject(error);
		});
	}

	// 实时视频
	static OpenVideoCtrl(fnPlay, fnClose = (div, e) => console.log(div)) {
		let own = this;
		const isPhoneBrw = CKcUtil.isPhoneBrower();
		// 动态创建控件
		let div = document.createElement("div");
		div._btnClose = document.createElement("a");
		div._btnFull = document.createElement("a");
		div._btnCtrl = document.createElement("a");
		div._video = document.createElement("video");
		// 外框控件
		div.style.position = "absolute";
		div.style.border = '1px solid silver';
		div.style.backgroundColor = "rgba(0, 0, 0, 0.5)";
		div.style.width = '480px';
		//div.style.height = '480px';
		div.style.top = div.style.left = '0px';
		div.style.zIndex = 99;
		div.style.overflow = "hidden";
		// 关闭按钮
		div._btnClose.innerHTML = "❌";
		div._btnClose.style.cursor = "pointer";
		div._btnClose.style.position = "absolute";
		div._btnClose.style.top = div._btnClose.style.right = '6px';
		div._btnClose.onclick = e => fnClose(div, e);
		// 全屏按钮
		div._btnFull.innerHTML = "☐️";
		div._btnFull.style.fontSize = "18px";
		div._btnFull.style.cursor = "pointer";
		div._btnFull.style.position = "absolute";
		div._btnFull.style.top = '3px';
		div._btnFull.style.right = '36px';
		div._btnFull.onclick = e => {
			if (div._video.webkitRequestFullscreen) div._video.webkitRequestFullscreen();
			else if (div._video.mozRequestFullScreen) div._video.mozRequestFullScreen();
			else if (div._video.msRequestFullscreen) div._video.msRequestFullscreen();
			else if (div._video.oRequestFullscreen) div._video.oRequestFullscreen();
			else if (div._video.requestFullscreen) div._video.requestFullscreen();
		};
		// 配置按钮
		div._btnCtrl.innerHTML = "🔒️️️";
		div._btnCtrl.style.fontSize = "18px";
		div._btnCtrl.style.cursor = "pointer";
		div._btnCtrl.style.position = "absolute";
		div._btnCtrl.style.top = '3px';
		div._btnCtrl.style.left = '6px';
		div._btnCtrl.onclick = e => {
			div._video.controls = !div._video.controls;
			div._btnCtrl.innerHTML = div._video.controls ? "🔓" : "🔒️️️";
		};
		// 动态创建视频控件
		div._video.controls = false;
		div._video.playsinline = true;
		div._video.autoplay = true;
		div._video.style.width = '100%';
		//div._video.style.height = '92%';
		div._video.style.position = "relative";
		div._video.style.top = '36px';
		div._video.style.bottom = '0px';
		div._video.style.zIndex = 101;
		//div._video.srcObject = e.streams[0];
		fnPlay(div._video, div);
		// 添加控件
		div.appendChild(div._btnClose);
		div.appendChild(div._btnFull);
		div.appendChild(div._btnCtrl);
		div.appendChild(div._video);
		document.body.appendChild(div);
		CKcUtil.ctrlCanMove(div);
		return div;
	}

	// Client唯一编号
	static g_KCCLNID = "";
	static getKCCLNID() {
		if (CKcUtil.isBlank(CKcUtil.g_KCCLNID))
			CKcUtil.g_KCCLNID = CKcUtil.getCookie("KCCLNID");
		if (CKcUtil.isBlank(CKcUtil.g_KCCLNID)) {
			CKcUtil.g_KCCLNID = CKcUtil.getLocalStorage("KCCLNID");
			if (!CKcUtil.isBlank(CKcUtil.g_KCCLNID))
				CKcUtil.setCookieByDays("KCCLNID", CKcUtil.g_KCCLNID, 365 * 100 + 24);
		}
		return CKcUtil.nvl(CKcUtil.g_KCCLNID, "");
	}
	static setKCCLNID(val) {
		CKcUtil.g_KCCLNID = val;
		CKcUtil.setLocalStorage("KCCLNID", val);
		CKcUtil.setCookieByDays("KCCLNID", val, 365 * 100 + 24);
	}
	// Session唯一编号
	static g_KCSSID = "";
	static getKCSSID() {
		// if (CKcUtil.isBlank(CKcUtil.g_KCSSID)) CKcUtil.g_KCSSID = CKcUtil.getCookie("KCSSID");
		if (CKcUtil.isBlank(CKcUtil.g_KCSSID)) CKcUtil.g_KCSSID = CKcUtil.getSessionStorage("KCSSID");
		return CKcUtil.nvl(CKcUtil.g_KCSSID, "");
	}
	static setKCSSID(val) {
		CKcUtil.g_KCSSID = val;
		CKcUtil.setSessionStorage("KCSSID", val);
		//CKcUtil.setCookie("KCSSID", val);
	}
	// url上的Client和Session编号
	static urlParmKCID() {
		let sGetParm = "";
		if (!CKcUtil.isBlank(CKcUtil.getKCCLNID()))
			sGetParm = "KCCLNID=" + CKcUtil.g_KCCLNID;
		if (!CKcUtil.isBlank(CKcUtil.getKCSSID())) {
			if (CKcUtil.isBlank(sGetParm))
				sGetParm = "KCSSID=" + CKcUtil.g_KCSSID;
			else
				sGetParm = sGetParm + "&KCSSID=" + CKcUtil.g_KCSSID;
		}
		return sGetParm;
	}
	// json的Client和Session编号
	static jsnParmKCID() {
		return {
			"KCSSID": CKcUtil.getKCSSID(),
			"KCCLNID": CKcUtil.getKCCLNID()
		};
	}

	// 消息事件
	static _default_event_name = "KcMsgEvent";
	_event_handlers = {};
	// 添加消息处理函数
	addEventListener(ename, fn) {
		try {
			//console.log("addEventListener[" + (new Date()).toLocaleString() + "] " + ename + "\t" + fn.toString());
			if (this._event_handlers[ename] === undefined)
				this._event_handlers[ename] = [];
			else this.removeEventListener(ename, fn);
			this._event_handlers[ename].push(fn);
		} catch (err) {
			console.error({
				"name": ename,
				"func": fn,
				"error": err
			});
		}
	}
	attachEvent(fn) {
		this.addEventListener(CKcUtil._default_event_name, fn);
	}
	// 移除消息处理函数
	removeEventListener(ename, fn) {
		try {
			//console.log("removeEventListener[" + (new Date()).toLocaleString() + "] " + ename + "\t" + fn.toString());
			let typeHandlers = this._event_handlers[ename];
			if (typeHandlers === undefined || typeHandlers.length === 0)
				return;
			for (let i = typeHandlers.length - 1; i >= 0; --i)
				if (typeHandlers[i] === fn) {
					typeHandlers.splice(i, 1);
					break;
				}
			if (typeHandlers.length == 0) delete this._event_handlers[ename]
		} catch (err) {
			console.error({
				"name": ename,
				"func": fn,
				"error": err
			});
		}
	}
	detachEvent(fn) {
		this.removeEventListener(CKcUtil._default_event_name, fn);
	}
	clearEventListener(ename) {
		try {
			let typeHandlers = this._event_handlers[ename];
			if (typeHandlers === undefined || typeHandlers.length === 0)
				return;
			typeHandlers.splice(0, typeHandlers.length);
			delete this._event_handlers[ename];
		} catch (err) {
			console.error({
				"name": ename,
				"error": err
			});
		}
	}

	// 触发消息事件
	dispatchEvent(ename, jsn) {
		let iResult = 0;
		try {
			//console.log("dispatchEvent[" + (new Date()).toLocaleString() + "] " + ename + "\n" + JSON.stringify(jsn));
			let typeHandlers = this._event_handlers[ename];
			if (typeHandlers === undefined || typeHandlers.length === 0)
				return iResult;
			for (let i = 0; i < typeHandlers.length; ++i)
				try {
					typeHandlers[i].apply(this, [jsn]);
					++iResult;
				}
			catch (err) {
				console.error({
					"name": ename,
					"data": jsn,
					"error": err,
					"iPos": i,
					"function": typeHandlers[i]
				});
			}
		} catch (err) {
			console.error({
				"name": ename,
				"data": jsn,
				"error": err
			});
		}
		return iResult;
	}
	fireEvent(jsn) {
		return this.dispatchEvent(CKcUtil._default_event_name, jsn);
	}

	// 构造函数
	constructor() {
		super();
		// 不能创建基类对象
		if (new.target === CKcUtil)
			throw new Error("Can't Create CKcUtil Object");
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		// 清除所有事件
		for (let evn in this._event_handlers)
			this.clearEventListener(evn);
	}
}
class CKcUtilEx extends CKcUtil {
	// 播放声音
	makeSound(src) {
		let own = this;
		setTimeout(function() {
			own._audio.src = src;
			//if (own._audio.load) own._audio.load();
			if (own._audio.play) own._audio.play();
		}, 66);
	}

	// 循环播放视频（移动设备下防止锁屏）
	makeHoldRun(src) {
		let own = this;
		setTimeout(function() {
			own._video._video.src = src;
			//if (own._video._video.load) own._video._video.load();
			if (own._video._video.play) own._video._video.play();
			own._video._video.playbackRate = 0.1;
		}, 66);
	}
	makeHoldStop() {
		this._video._video.pause();
		own._video._video.src = null;
	}

	// 构造函数
	constructor() {
		super();

		// 声音
		this._audio = new Audio();
		this._audio.volume = 1;

		// 视频控件
		this._video = CKcUtil.OpenVideoCtrl((video, div) => {
			video.style.top = "0";
			video.loop = video.muted = true;
			//video.src = require('@/assets/video/holdplay.mp4');
			//video.src = 'https://86my.cn/res/holdplay.mp4';
			//video.playbackRate = 0.1;
			div._btnClose.style.display = div._btnFull.style.display = div._btnCtrl.style.display = "none";
			div.style.left = '';
			div.style.right = '0px';
			div.style.border = '0px solid silver';
			div.style.backgroundColor = "rgba(0, 0, 0, 0)";
			//div.style.display = "none";
			//div.style.height = "16px";
			div.style.width = "24px";
		});
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

//////////////////////////////////////////////////////////////////////// ajax //////////////////////////////////////////////////////////////////
// ajax类
class CKcAjax extends CKcUtil {
	// 调用的顺序号
	static g_CallSortID = 0;

	// 发送成功事件
	onsuccess = function(id, json) {
		console.log(json);
	}
	// 发送失败事件
	onfailure = function(id, err) {
		console.log(err);
	}

	// 请求
	request(id, isPost, isFormData, uri, prm) {
		let own = this
		let data = isPost && !isFormData ? JSON.stringify(prm) : prm;
		let xhr = new XMLHttpRequest();
		xhr.withCredentials = true;
		xhr.open(isPost ? 'POST' : 'GET', uri, true); // get请求，请求地址，是否异步
		//xhr.setRequestHeader("Cookie", "KCSSID=123");
		xhr.responseType = "text"; // 返回类型text
		//xhr.responseType = "blob";  	// 返回类型blob
		xhr.onload = function() { // 请求完成处理函数
			if (this.status === 200) {
				try {
					//let ck = xhr.getAllResponseHeaders();
					let jsn = JSON.parse(xhr.response);
					if (!CKcUtil.hasOwnProp(jsn, "errMsg"))
						jsn.errMsg = "";
					if (!CKcUtil.hasOwnProp(jsn, "errCode"))
						jsn.errCode = 0;
					if (jsn.errCode === 0) {
						own.onsuccess(id, jsn);
						own.dispatchEvent("success", jsn);
					} else
						own.onfailure(id, jsn);
				} catch (err) {
					own.onfailure(id, {
						"errCode": 998,
						"errMsg": err,
						"uri": uri,
						"request": data,
						"response": xhr.response
					});
				}
			} else
				own.onfailure(id, {
					"errCode": this.status,
					"errMsg": xhr.response
				});
		};
		xhr.onerror = function(e) {
			own.onfailure(id, {
				"errCode": 999,
				"errMsg": "Unknown Error",
				"event": e
			});
		};
		if (isPost)
			xhr.send(data);
		else
			xhr.send();
	}
	// get请求
	get(id, uri) {
		this.request(id, false, false, uri, null);
	}
	// post请求
	post(id, uri, prm) {
		this.request(id, true, false, uri, prm);
	}
	// 多表单请求
	postfd(id, uri, fd) {
		this.request(id, true, true, uri, fd);
	}
	// 请求
	static _Exec(uri, prm, files) {
		return new Promise(function(resolve, reject) {
			let iSortID = ++CKcAjax.g_CallSortID;
			try {
				// 本地client和session编号
				const localKCSSID = CKcUtil.getKCSSID();
				const localKCCLNID = CKcUtil.getKCCLNID();
				// 携带本地client和session编号
				if (CKcUtil.isNull(prm)) prm = {
					"KCSSID": localKCSSID,
					"KCCLNID": localKCCLNID
				};
				else {
					prm.KCSSID = localKCSSID;
					prm.KCCLNID = localKCCLNID;
				}
				// ajax对象
				let aj = new CKcAjax;
				// 发送失败事件
				aj.onfailure = (i, err) => reject(err);
				// 发送成功事件
				aj.onsuccess = (i, jsn) => {
					// 服务器端session编号
					if (CKcUtil.hasOwnProp(jsn, "KCSSID") && localKCSSID != jsn.KCSSID)
						CKcUtil.setKCSSID(jsn.KCSSID);
					// 服务器端client编号
					if (CKcUtil.hasOwnProp(jsn, "KCCLNID") && localKCCLNID != jsn.KCCLNID)
						CKcUtil.setKCCLNID(jsn.KCCLNID);
					// 成功返回
					resolve(jsn);
				}
				// 上传文件封装为多表单数据
				if (files) {
					var formData = new FormData();
					formData.append('json', JSON.stringify(prm))
					for (let i = 0; i < files.length; ++i)
						formData.append('file', files[i])
					aj.postfd(iSortID, uri, formData);
				}
				// 发送json请求
				else aj.post(iSortID, uri, prm);
			} catch (err) {
				reject({
					errCode: iSortID,
					errMsg: err
				});
			}
		});
	}
	static Exec(uri, prm) {
		return this._Exec(uri, prm, null);
	}
	static Upload(uri, prm, files) {
		return this._Exec(uri, prm, files);
	}

	// 构造函数
	constructor() {
		super();
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
	}
}

//////////////////////////////////////////////////////////////////////// Server-Sent Events //////////////////////////////////////////////////////////////////
// SSE类
class CKcSSE extends CKcUtil {
	// 链接编号
	sid = "";

	// 链接开始事件
	onbegin = function(sid) {
		console.log({
			"SSE_onbegin": sid
		});
	}
	// 连接失败事件
	onfailure = function(sid, err) {
		console.error({
			"SSE_onfailure": sid,
			"error": err
		});
	}
	// 错误事件
	onerr = function(sid, err) {
		console.error({
			"SSE_onerr": sid,
			"error": err
		});
	}
	// 警告事件
	onwarn = function(sid, warn) {
		console.log({
			"SSE_onwarn": sid,
			"warn": warn
		});
	}
	// 消息事件（如果未指定“事件名称”）
	onmsg = function(sid, json) {
		console.log({
			"SSE_onmsg": sid,
			"data": json
		});
	}

	// 判断是否连接
	isConnect() {
		return CKcUtil.hasOwnProp(this, "_sse") && !CKcUtil.isNull(this._sse);
	}

	// 函数：关闭链接
	close() {
		try {
			if (this._sse) {
				this._sse.close();
				delete this._sse;
			}
			this.sid = "";
		} catch (err) {
			console.error("SSE_close", this.sid, err);
		}
	}

	// 函数：链接
	connect() {
		this.sid = "";
		let sUrlParmKCID = CKcUtil.urlParmKCID();
		if (CKcUtil.isBlank(sUrlParmKCID))
			// throw new Error('KCSSID is null');
			this.onfailure(this.sid, {
				errCode: 1,
				errMsg: "KCSSID is null",
				uri: this._uri
			});
		let sUri = CKcUtil.addParmAtUrl(this._uri, sUrlParmKCID)
		if (!CKcUtil.isBlank(sUri)) {
			//console.log("SSE_connect", sUri);
			let own = this;
			// sse
			this._sse = new EventSource(sUri, {
				withCredentials: true
			});
			// sse事件：打开事件
			this._sse.onopen = function(e) {
				//console.log("SSE_onopen", sUri, e);
			}
			// 连接失败事件
			this._sse.onerror = function(e) {
				own.onfailure(own.sid, {
					errCode: 2,
					errMsg: CKcUtil.isNull(e.data) ? "Unknown Error" : e.data,
					uri: sUri,
					event: e
				});
			}
			// 消息事件
			this._sse.onmessage = function(e) {
				let jsn = JSON.parse(e.data);
				if (!CKcUtil.hasOwnProp(jsn, "sid")) jsn.sid = own.sid;
				// 触发事件
				if (!CKcUtil.hasOwnProp(jsn, "event") || own.dispatchEvent(jsn.event, jsn) == 0)
					// 普通消息
					own.onmsg(own.sid, jsn);
			}
			// BeginWork 事件
			this._sse.addEventListener('BeginWork', function(e) {
				own.sid = e.data;
				// 其他错误事件
				own._sse.onerror = e => own.onerr(own.sid, {
					errCode: 1,
					errMsg: CKcUtil.isNull(e.data) ? "Unknown Error" : e.data,
					event: e
				});
				// 触发开始事件
				own.onbegin(own.sid);
			}, false);
			// ServerError 事件
			this._sse.addEventListener('ServerError', function(e) {
				own.onerr(own.sid, {
					errCode: 2,
					errMsg: e.data
				});
				own.close();
			}, false);
			// Warning 事件
			this._sse.addEventListener('Warning', function(e) {
				own.onwarn(own.sid, {
					errCode: 100,
					errMsg: e.data
				});
			}, false);
		}
	}

	// 构造函数
	constructor(uri) {
		super();
		this._uri = uri;
		this.connect();
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.close();
	}
}

//////////////////////////////////////////////////////////////////////// websocket //////////////////////////////////////////////////////////////////
// websocket类
class CKcWS extends CKcUtil {
	// 链接编号
	sid = "";

	// 链接开始事件
	onbegin = function(sid) {
		console.log({
			"WS_onbegin": sid
		});
	}
	// 链接结束事件
	onend = function(sid) {
		console.log({
			"WS_onend": sid
		});
	}
	// 错误事件
	onerr = function(sid, err) {
		console.error({
			"WS_onend": sid,
			"error": err
		});
	}
	// 警告事件
	onwarn = function(sid, warn) {
		console.log({
			"WS_onend": sid,
			"warn": warn
		});
	}
	// 消息事件（如果未指定“事件名称”）
	onmsg = function(sid, json) {
		console.log({
			"WS_onend": sid,
			"data": json
		});
	}

	// 函数：关闭链接
	close() {
		try {
			this._uri = "";
			this._ws.close();
		} catch (err) {
			console.error({
				"WS_close": this.sid,
				"error": err
			});
		}
	}

	// 函数：发送消息
	send(json) {
		try {
			if (this._ws.readyState == 1)
				this._ws.send(JSON.stringify(json));
		} catch (err) {
			console.error({
				"WS_send": this.sid,
				"data": json,
				"error": err
			});
		}
	}

	// 函数：链接
	connect() {
		if (!CKcUtil.isBlank(this._uri)) {
			console.log({
				"WS_connect": this._uri
			});
			let own = this;
			// websocket
			this._ws = new WebSocket(this._uri);
			// 心跳
			this._heartCheck = {
				timeout: 6666,
				timeoutObj: null,
				start: function() {
					this.stop();
					this.timeoutObj = setTimeout(function() {
						own.send({
							"ping": own.sid
						});
						own._heartCheck.start();
					}, this.timeout)
				},
				stop: function() {
					if (!CKcUtil.isNull(this.timeoutObj)) clearTimeout(this.timeoutObj);
				}
			}
			// websocket事件
			this._ws.onopen = function(e) {
				//当WebSocket创建成功时，触发onopen事件
				console.log({
					"WS_onopen": own._uri,
					"event": e
				});
				//this._ws.send("hello"); //将消息发送到服务端
				own._heartCheck.start();
			}
			this._ws.onclose = function(e) {
				own._heartCheck.stop();
				//当客户端收到服务端发送的关闭连接请求时，触发onclose事件
				console.log({
					"WS_onclose": own._uri,
					"event": e
				});
				own.onend(own.sid);
				(async function() {
					await CKcUtil.sleep(666);
					own.reConnect();
				})();
			}
			this._ws.onerror = function(e) {
				//如果出现连接、处理、接收、发送数据失败的时候触发onerror事件
				console.error({
					"WS_onerror": own._uri,
					"event": e
				});
				own.onerr(own.sid, {
					errCode: 1,
					errMsg: e.data
				});
				//if (own._ws.readyState != 1) CKcUtil.sleep(1666).then(() => {own.reConnect();});
			}
			this._ws.onmessage = function(e) {
				try {
					//当客户端收到服务端发来的消息时，触发onmessage事件，参数e.data包含server传递过来的数据
					//console.log("onmessage[" + (new Date()).toLocaleString() + "] " + e.data);
					let jsn = JSON.parse(e.data);
					// 链接失败
					if (CKcUtil.hasOwnProp(jsn, "errWithCloseWSConn") && Object.keys(jsn).length == 1)
						own.onerr(own.sid, {
							errCode: 2,
							errMsg: jsn.errWithCloseWSConn
						});
					// 链接成功，记录链接编号
					else if ("sid" in jsn && Object.keys(jsn).length == 1) {
						own.sid = jsn.sid;
						own.onbegin(own.sid);
					}
					// 警告
					else if (CKcUtil.hasOwnProp(jsn, "Warning") && Object.keys(jsn).length == 1) {
						own.onwarn(own.sid, jsn);
						//if ("PreClose" == jsn.Warning) own._ws.close();	// 断开的警告，重连
						//if ("PreClose" == jsn.Warning) own._ws.close();	// 断开的警告，重连
					}
					// 服务器推送的消息
					else {
						jsn.sid = own.sid;
						if (!CKcUtil.hasOwnProp(jsn, "event") || own.dispatchEvent(jsn.event, jsn) == 0)
							own.onmsg(own.sid, jsn);
					}
				} catch (err) {
					console.error({
						"WS_onmessage": own._uri,
						"error": err,
						"event": e
					});
					own.onerr(own.sid, {
						errCode: 3,
						errMsg: err
					});
				}
			}
		}
	}

	// 函数：重连
	reConnect() {
		if (this._lockReconnect) return;
		this._lockReconnect = true;
		let own = this;
		setTimeout(function() {
			own.connect();
			own._lockReconnect = false;
		}, 6666);
	}

	// 构造函数
	constructor(uri) {
		super();
		this._lockReconnect = false;
		this._uri = uri;
		this.connect();
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this.close();
	}
}

//////////////////////////////////////////////////////////////////////// 服务器中转 //////////////////////////////////////////////////////////////////
// 服务器中转数据（如，聊天信息等）类
class CKcSrvTransfer extends CKcUtil {
	// 事件名称
	_srv_trans_event_name = "SrvTransEvent_";

	// 接收事件
	onrecv = function(jsn) {
		console.log("Trans_onrecv", this._eventRecvName, this._objSSE.sid, jsn);
	}

	// 构造函数
	constructor(objSSE, myFlag = "talk") {
		super();
		let own = this;

		// 接收事件
		this._eventRecvName = this._srv_trans_event_name + myFlag;
		this._eventRecvFN = data => own.onrecv(data);

		// SSE对象
		this._objSSE = objSSE;
		this._objSSE.addEventListener(this._eventRecvName, this._eventRecvFN);
	}
	// 析构函数（需手工调用）
	destructor() {
		super.destructor();
		this._objSSE.removeEventListener(this._eventRecvName, this._eventRecvFN);
	}

	// 发送（toFlag要与接收方的myFlag一致）
	SendData(ajaxUri, json, toFlag = "talk") {
		// 发送事件
		json.event = this._srv_trans_event_name + toFlag;
		// 发送方SSE编号
		json.from = this._objSSE.sid;
		// 发送请求
		return CKcAjax.Exec(ajaxUri, json);
	}
}
