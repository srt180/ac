#include <mongoose.h>
#include <string>
#include <mutex>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <thread>
#include <spdlog/spdlog.h>
#include "util.h"

using json = nlohmann::json;

#ifndef AC_WS_H
#define AC_WS_H

#define IDENTITY_SUCCESS 200
#define IDENTITY_NOT_FOUND 400
#define IDENTITY_WRONG_STATION 403
#define IDENTITY_NOT_TIME 404

#define OTHER_GARBAGE 0
#define WET_GARBAGE 1

class wsConn {
public:
    static wsConn* getInstance(const std::string &name) {
        static std::map<std::string, wsConn*> instanceList;
        auto search = instanceList.find(name);
        if (search != instanceList.end()) {
            auto conn = search->second;
            if (!conn->isConnected()) {
                spdlog::debug("ws reconnect");
                conn->connect(conn->url);
            }
            return conn;
        } else {
            auto newWSconn = new wsConn(name);
            instanceList.emplace(std::make_pair(name, newWSconn));
            return newWSconn;
        }
    }

    int connected = 0;  // 0: unconnected 1: connected 2: connecting
    int done = 0;
    bool isConnected();

    // as client
    void connect(const std::string &url);
    bool send(const std::string&);

    //as server
    bool bindAndServe(const char *port, MG_CB(mg_event_handler_t handler,
                                              void *user_data));
    void emplace_back_connection(mg_connection* nc);
    bool erase_connection(mg_connection* c);
    void sendToAll(const char *msg);

    //as server end
    void close();

    static void backend_ev_handler(struct mg_connection *nc, int ev, void *ev_data);
    static void machine_ev_handler(struct mg_connection *nc, int ev, void *ev_data);

    mg_mgr* GetMgr();

private:
    std::string instanceName;
    std::string url;
    wsConn(){};
    wsConn(const std::string &name){this->instanceName = name;};

    class GC {
    public:
        ~GC(){}
    };
    static GC gc;

    class Citizen {
        std::string name{""};
        std::string cardNo{""};
        int point{0};
        int timestamp{0};
        PERSON_STATUS status{PERSON_EXIT};

    public:
        std::mutex mtx;

        const std::string &getName() const;
        void setName(const std::string &name);
        const std::string &getCardNo() const;
        void setCardNo(const std::string &cardNo);
        int getPoint() const;
        void setPoint(int point);
        int getTimestamp() const;
        void setTimestamp(int timestamp);
        void setTimestampNow();
        PERSON_STATUS getStatus() const;
        void setStatus(PERSON_STATUS status);

        void checkLock() ;

        json formJSON();
    };


    std::vector<mg_connection*> clientList;

    mg_connection* nc = nullptr;
    mg_mgr *mgr = nullptr;



public:
    static std::mutex mtx;
    static Citizen* getCitizen() {
        //mtx.lock();
        static Citizen citizen;
        return &citizen;
    }
    static void unlock() {
       // mtx.unlock();
    }

};

#endif //AC_WS_H