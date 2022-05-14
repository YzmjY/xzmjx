#include "chat_protocol.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
namespace chat{
ChatMessage::ChatMessage(){

}

ChatMessage::ptr ChatMessage::Create(const std::string& msg){
    rapidjson::Document doc;
    if(!(doc.Parse(msg.c_str()).HasParseError())){
        ChatMessage::ptr chat_msg(new ChatMessage);
        for(auto it = doc.MemberBegin();
                it != doc.MemberEnd();++it){
            chat_msg->set(it->name.GetString(),it->value.GetString());
        }
        return chat_msg;
    }
    return nullptr;
}

std::string ChatMessage::get(const std::string& id) const{
    auto it = m_data.find(id);
    return it == m_data.end()?"":it->second;
}

void ChatMessage::set(const std::string& id,const std::string& val){
    m_data[id] = val;
}

std::string ChatMessage::toString() const{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer;
    writer.StartObject();
    for(auto&& it:m_data){
        writer.Key(it.first.c_str());
        writer.String(it.second.c_str());
    }
    writer.EndObject();
    return buf.GetString();
}
}