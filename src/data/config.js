// Work out the endpoint to use, for dev you can change to point at a remote ESP
// and run the HTML/JS from file, no need to upload to the ESP to test

var baseHost = window.location.hostname;
//var baseHost = 'emonesp.local';
//var baseHost = '192.168.4.1';
//var baseHost = '192.168.1.117';
var baseEndpoint = 'http://' + baseHost;

var statusupdate = false;
var selected_network_ssid = "";
var ipaddress = "";

// Convert string to number, divide by scale, return result
// as a string with specified precision

function recTime(tVal) {
    var out = '';
    var a = 0,
        b = 0;
    a = (parseInt(tVal) - parseInt(tVal) % 100) / 100;
    b = parseInt(tVal) % 100;
    if (a < 10)
        out += '0';
    out += a;
    out += ':';
    if (b < 10)
        out += '0';
    out += b;
    return out;
}

function lineTime(p0, p1) {
    var a, b;
    a = p0;
    b = p1;
    var temp = new Array();
    temp[0] = valPres(a / 2359 * 100, 2);
    temp[1] = valPres((b - a) / 2359 * 100, 2);
    temp[2] = valPres((2359 - b) / 2359 * 100, 2);
    return temp;
}


function scaleString(string, scale, precision) {
    var tmpval = parseInt(string) / scale;
    return tmpval.toFixed(precision);
}

function valPres(val, precision) {
    return val.toFixed(precision);
}

function scaleByte(val) {
    if (val >= 1048576) {
        return Math.round(100 * val / 1048576) / 100 + " MB";
    } else if (val >= 1024) {
        return Math.round(100 * val / 1024) / 100 + " kB";
    } else {
        return val + " B";
    }
}

function freeDisk(d_total, d_used) {
    var temp = "";
    temp = scaleByte(d_total - d_used);
    return temp;
}

function calcDBM(val) {
    if (val < 0) {
        return 100 + val;
    } else {
        return val;
    }
}

function BaseViewModel(defaults, remoteUrl, mappings) {
    if (mappings === undefined) {
        mappings = {};
    }
    var self = this;
    self.remoteUrl = remoteUrl;

    // Observable properties
    ko.mapping.fromJS(defaults, mappings, self);
    self.fetching = ko.observable(false);
}

BaseViewModel.prototype.update = function (after) {
    if (after === undefined) {
        after = function () {};
    }
    var self = this;
    self.fetching(true);
    $.get(self.remoteUrl, function (data) {
        ko.mapping.fromJS(data, self);
    }, 'json').always(function () {
        self.fetching(false);
        after();
    });
};

function StatusViewModel() {
    var self = this;

    BaseViewModel.call(self, {
        "mode": "",
        "networks": [],
        "rssi": [],
        "c_srssi": 0,
        "c_ssid": "",
        "ipaddress": "",
        "emon_connected": 0,
        "packets_sent": 0,
        "packets_success": 0,
        "mqtt_connected": 0,
        "current_t": 0,
        "target_t": 0,
        "free_heap": 0,
        "total_bytes": 0,
        "used_bytes": 0,
        "time_hh": 0,
        "time_mm": 0,
        "time_ss": 0,
        "date_dd": 0,
        "date_mm": 0,
        "date_yy": 0,
        "date_dw": 0
    }, baseEndpoint + '/status');

    // Some devired values
    self.showTime = ko.pureComputed(function () {
        var out = '';
        if (self.time_hh() < 10)
            out += '0';
        out += self.time_hh();
        out += '.';
        if (self.time_mm() < 10)
            out += '0';
        out += self.time_mm();
        return out;
    });

    self.showDate = ko.pureComputed(function () {
        var out = '';
        var mnt = ["JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"];
        var dow = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
        out += dow[self.date_dw()];
        out += ', ';
        out += mnt[self.date_mm() - 1];
        out += ' ';
        out += self.date_dd();
        return out;
    });

    self.emonIcon = ko.pureComputed(function () {
        if (self.emon_connected())
            return 'sync';
        else
            return 'sync_problem';
    });
    self.mqttMsg = ko.pureComputed(function () {
        if (self.mqtt_connected())
            return 'Connected';
        else
            return '---';
    });
    self.mqttIcon = ko.pureComputed(function () {
        if (self.mqtt_connected())
            return 'sync';
        else
            return 'sync_problem';
    });
    self.emonRate = ko.pureComputed(function () {
        if (self.emon_connected)
            return 100 * self.packets_success() / self.packets_success() + ' %';
        else
            return '---';
    });
    self.isWifiClient = ko.pureComputed(function () {
        return ("STA" == self.mode()) || ("STA+AP" == self.mode());
    });
    self.isWifiAccessPoint = ko.pureComputed(function () {
        return ("AP" == self.mode()) || ("STA+AP" == self.mode());
    });
    self.fullMode = ko.pureComputed(function () {
        switch (self.mode()) {
            case "AP":
                return "Access Point (AP)";
            case "STA":
                return "Client (STA)";
            case "STA+AP":
                return "Client + Access Point (STA+AP)";
        }

        return "Unknown (" + self.mode() + ")";
    });
}
StatusViewModel.prototype = Object.create(BaseViewModel.prototype);
StatusViewModel.prototype.constructor = StatusViewModel;

function ConfigViewModel() {
    BaseViewModel.call(this, {
        "ssid": "",
        "pass": "",
        "emon_server": "",
        "emon_node": "",
        "emon_apikey": "",
        "emon_fingerprint": "",
        "emon_interval": 0,
        "mqtt_server": "",
        "mqtt_topic": "",
        "mqtt_feed_prefix": "",
        "mqtt_user": "",
        "mqtt_pass": "",
        "mqtt_interval": 0,
        "mqtt_port": 0,
        "www_username": "",
        "www_password": "",
        "ntp_ip1": 0,
        "ntp_ip2": 0,
        "ntp_ip3": 0,
        "ntp_ip4": 0,
        "ntp_tz": 0,
        "version": ""
    }, baseEndpoint + '/config');
}
ConfigViewModel.prototype = Object.create(BaseViewModel.prototype);
ConfigViewModel.prototype.constructor = ConfigViewModel;

function ScheduleViewModel() {
    BaseViewModel.call(this, {
        "n": 0,
        "sp_time": [],
        "sp": []
    }, baseEndpoint + '/schedule');
}
ScheduleViewModel.prototype = Object.create(BaseViewModel.prototype);
ScheduleViewModel.prototype.constructor = ScheduleViewModel;



function EmonViewModel() {
    var self = this;

    self.schedule = new ScheduleViewModel();
    self.config = new ConfigViewModel();
    self.status = new StatusViewModel();

    self.initialised = ko.observable(false);
    self.updating = ko.observable(false);

    var updateTimer = null;
    var updateTime = 1 * 1000;

    // Upgrade URL
    self.upgradeUrl = ko.observable('about:blank');

    // -----------------------------------------------------------------------
    // Initialise the app
    // -----------------------------------------------------------------------
    self.start = function () {
        self.updating(true);
        self.schedule.update(function () {
            self.config.update(function () {
                self.status.update(function () {
                    self.initialised(true);
                    updateTimer = setTimeout(self.update, updateTime);
                    self.upgradeUrl(baseEndpoint + '/update');
                    self.updating(false);
                });
            });
        });
    };

    // -----------------------------------------------------------------------
    // Get the updated state from the ESP
    // -----------------------------------------------------------------------
    self.update = function () {
        if (self.updating()) {
            return;
        }
        self.updating(true);
        if (null !== updateTimer) {
            clearTimeout(updateTimer);
            updateTimer = null;
        }
        self.status.update(function () {
            updateTimer = setTimeout(self.update, updateTime);
            self.updating(false);
        });
    };

    self.wifiConnecting = ko.observable(false);
    self.status.mode.subscribe(function (newValue) {
        if (newValue === "STA+AP" || newValue === "STA") {
            self.wifiConnecting(false);
        }
    });
    // -----------------------------------------------------------------------
    // Event: SP Plus
    // -----------------------------------------------------------------------
    self.SPPlusFetching = ko.observable(false);
    self.SPPlusSuccess = ko.observable(false);
    self.SPPlus = function () {
        self.SPPlusFetching(true);
        self.SPPlusSuccess(false);
        $.post(baseEndpoint + "/tplus", function (data) {
            self.SPPlusSuccess(true);
        }).fail(function () {
            alert("Failed to incrase SP");
        }).always(function () {
            self.SPPlusFetching(false);
        });
    };
    // -----------------------------------------------------------------------
    // Event: SP Minus
    // -----------------------------------------------------------------------
    self.SPMinusFetching = ko.observable(false);
    self.SPMinusSuccess = ko.observable(false);
    self.SPMinus = function () {
        self.SPMinusFetching(true);
        self.SPMinusSuccess(false);
        $.post(baseEndpoint + "/tminus", function (data) {
            self.SPMinusSuccess(true);
        }).fail(function () {
            alert("Failed to decrase SP");
        }).always(function () {
            self.SPMinusFetching(false);
        });
    };
    // -----------------------------------------------------------------------
    // Event: WiFi Connect
    // -----------------------------------------------------------------------
    self.saveNetworkFetching = ko.observable(false);
    self.saveNetworkSuccess = ko.observable(false);
    self.saveNetwork = function () {
        if (self.config.ssid() === "") {
            alert("Please select network");
        } else {
            self.saveNetworkFetching(true);
            self.saveNetworkSuccess(false);
            $.post(baseEndpoint + "/savenetwork", {
                ssid: self.config.ssid(),
                pass: self.config.pass()
            }, function (data) {
                self.saveNetworkSuccess(true);
                self.wifiConnecting(true);
            }).fail(function () {
                alert("Failed to save WiFi config");
            }).always(function () {
                self.saveNetworkFetching(false);
            });
        }
    };

    // -----------------------------------------------------------------------
    // Event: Admin save
    // -----------------------------------------------------------------------
    self.saveAdminFetching = ko.observable(false);
    self.saveAdminSuccess = ko.observable(false);
    self.saveAdmin = function () {
        self.saveAdminFetching(true);
        self.saveAdminSuccess(false);
        $.post(baseEndpoint + "/saveadmin", {
            user: self.config.www_username(),
            pass: self.config.www_password()
        }, function (data) {
            self.saveAdminSuccess(true);
        }).fail(function () {
            alert("Failed to save Admin config");
        }).always(function () {
            self.saveAdminFetching(false);
        });
    };

    // -----------------------------------------------------------------------
    // Event: NTP save
    // -----------------------------------------------------------------------
    self.saveNTPFetching = ko.observable(false);
    self.saveNTPSuccess = ko.observable(false);
    self.saveNTP = function () {
        var ntp = {
            ip1: self.config.ntp_ip1(),
            ip2: self.config.ntp_ip2(),
            ip3: self.config.ntp_ip3(),
            ip4: self.config.ntp_ip4(),
            tz: self.config.ntp_tz()
        };

        self.saveNTPFetching(true);
        self.saveNTPSuccess(false);
        $.post(baseEndpoint + "/saventp", ntp, function (data) {
            self.saveNTPSuccess(true);
        }).fail(function () {
            alert("Failed to save NTP config");
        }).always(function () {
            self.saveNTPFetching(false);
        });

    };
    // -----------------------------------------------------------------------
    // Event: Emon save
    // -----------------------------------------------------------------------
    self.saveEmonFetching = ko.observable(false);
    self.saveEmonSuccess = ko.observable(false);
    self.saveEmon = function () {
        var emon = {
            server: self.config.emon_server(),
            apikey: self.config.emon_apikey(),
            node: self.config.emon_node(),
            fingerprint: self.config.emon_fingerprint(),
            interval: self.config.emon_interval()
        };

        if (emon.server === "" || emon.node === "") {
            alert("Please enter EMON server and node");
        } else if (emon.apikey.length != 32) {
            alert("Please enter valid EMON apikey");
        } else if (emon.fingerprint !== "" && emon.fingerprint.length != 59) {
            alert("Please enter valid SSL SHA-1 fingerprint");
        } else {
            self.saveEmonFetching(true);
            self.saveEmonSuccess(false);
            $.post(baseEndpoint + "/saveemon", emon, function (data) {
                self.saveEmonSuccess(true);
            }).fail(function () {
                alert("Failed to save Admin config");
            }).always(function () {
                self.saveEmonFetching(false);
            });
        }
    };

    // -----------------------------------------------------------------------
    // Event: MQTT save
    // -----------------------------------------------------------------------
    self.saveMqttFetching = ko.observable(false);
    self.saveMqttSuccess = ko.observable(false);
    self.saveMqtt = function () {
        var mqtt = {
            server: self.config.mqtt_server(),
            topic: self.config.mqtt_topic(),
            prefix: self.config.mqtt_feed_prefix(),
            user: self.config.mqtt_user(),
            pass: self.config.mqtt_pass(),
            port: self.config.mqtt_port(),
            interval: self.config.mqtt_interval()
        };

        if (mqtt.server === "") {
            alert("Please enter MQTT server");
        } else {
            self.saveMqttFetching(true);
            self.saveMqttSuccess(false);
            $.post(baseEndpoint + "/savemqtt", mqtt, function (data) {
                self.saveMqttSuccess(true);
            }).fail(function () {
                alert("Failed to save MQTT config");
            }).always(function () {
                self.saveMqttFetching(false);
            });
        }
    };


    // -----------------------------------------------------------------------
    // Event: Schedule save
    // -----------------------------------------------------------------------
    self.saveScheduleFetching = ko.observable(false);
    self.saveScheduleSuccess = ko.observable(false);
    self.saveSchedule = function () {
        var schedule = {
            sp_time: self.schedule.sp_time(),
            sp: self.schedule.sp()
        };
        self.saveScheduleFetching(true);
        self.saveScheduleSuccess(false);
        $.post(baseEndpoint + "/saveschedule", schedule, function (data) {
            self.saveScheduleSuccess(true);
        }).fail(function () {
            alert("Failed to save Schedule config");
        }).always(function () {
            self.saveScheduleFetching(false);
        });
    };




}
$(function () {
    // Activates knockout.js
    var eaglemon = new EmonViewModel();
    ko.applyBindings(eaglemon);
    eaglemon.start();
});

// -----------------------------------------------------------------------
// Event: Turn off Access Point
// -----------------------------------------------------------------------
document.getElementById("apoff").addEventListener("click", function (e) {

    var r = new XMLHttpRequest();
    r.open("POST", "apoff", true);
    r.onreadystatechange = function () {
        if (r.readyState != 4 || r.status != 200)
            return;
        var str = r.responseText;
        console.log(str);
        document.getElementById("apoff").style.display = 'none';
        if (ipaddress !== "")
            window.location = "http://" + ipaddress;
    };
    r.send();
});
// -----------------------------------------------------------------------
// Event: Reset config and reboot
// -----------------------------------------------------------------------
document.getElementById("reset").addEventListener("click", function (e) {

    if (confirm("CAUTION: Do you really want to Factory Reset? All setting and config will be lost.")) {
        var r = new XMLHttpRequest();
        r.open("POST", "reset", true);
        r.onreadystatechange = function () {
            if (r.readyState != 4 || r.status != 200)
                return;
            var str = r.responseText;
            console.log(str);
            if (str !== 0)
                document.getElementById("reset").innerHTML = "Resetting...";
        };
        r.send();
    }
});
// -----------------------------------------------------------------------
// Event: Restart
// -----------------------------------------------------------------------
document.getElementById("restart").addEventListener("click", function (e) {

    if (confirm("Restart E-MON? Current config will be saved, takes approximately 10s.")) {
        var r = new XMLHttpRequest();
        r.open("POST", "restart", true);
        r.onreadystatechange = function () {
            if (r.readyState != 4 || r.status != 200)
                return;
            var str = r.responseText;
            console.log(str);
            if (str !== 0)
                document.getElementById("reset").innerHTML = "Restarting";
        };
        r.send();
    }
});
