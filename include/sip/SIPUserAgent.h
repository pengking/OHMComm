/* 
 * File:   SIPUserAgent.h
 * Author: daniel
 *
 * Created on February 24, 2016, 11:58 AM
 */

#ifndef SIPUSERAGENT_H
#define	SIPUSERAGENT_H

#include <string>
#include <map>

namespace ohmcomm
{
    namespace sip
    {

        /*!
         * Information about a participant of a session relevant for the SIP protocol
         */
        struct SIPUserAgent
        {
            std::string tag;
            std::string userName;
            std::string hostName;
            std::string ipAddress;
            int64_t associatedSSRC;
            unsigned short port;
            //the Call-ID associated with the conversation with this particular UA
            std::string callID;
            uint32_t sequenceNumber;
            //the value of the last branch-identifier, required for responses
            std::string lastBranch;

            SIPUserAgent(const std::string& tag) : tag(tag), userName(), hostName(), ipAddress(), associatedSSRC(-1), port(0),
            callID(), sequenceNumber(0), lastBranch()
            {

            }

            /*!
             * \return the SIP-URI in the format sip:<userName>@<hostName|ipAddress>[:<port>]
             */
            const inline std::string getSIPURI() const
            {
                //sip:user@host[:port]
                return (std::string("sip:") + userName + "@") + (hostName.empty() ? ipAddress : hostName) + (port > 0 ? std::string(":") + std::to_string(port) : std::string());
            }
        };

        /*!
         * Local database for all active user agents
         */
        class UserAgentDatabase
        {
        public:
            SIPUserAgent thisUA;

            UserAgentDatabase(const std::string& tag) : thisUA(tag)
            {

            }

            inline SIPUserAgent& getRemoteUA(const std::string& tag = "")
            {
                if (!isInUserAgentDB(tag))
                    remoteAgents.insert(std::pair<std::string, SIPUserAgent>(tag, SIPUserAgent(tag)));
                return remoteAgents.at(tag);
            }

            inline bool isInUserAgentDB(const std::string& tag) const
            {
                return remoteAgents.find(tag) != remoteAgents.end();
            }
            
            inline bool removeRemoteUA(const std::string& tag)
            {
                return remoteAgents.erase(tag) > 0;
            }
            
            SIPUserAgent* findForSSRC(const uint32_t ssrc)
            {
                auto it = remoteAgents.begin();
                auto end = remoteAgents.end();
                while(it != end)
                {
                    if((*it).second.associatedSSRC == ssrc)
                        return &((*it).second);
                    ++it;
                }
                return nullptr;
            }
            
            SIPUserAgent* findForAddress(const std::string& address, const uint16_t port)
            {
                auto it = remoteAgents.begin();
                auto end = remoteAgents.end();
                while(it != end)
                {
                    if((*it).second.ipAddress.compare(address) == 0)
                        //XXX we need to compare port too (especially for NATs), but RTP-ports not SIP-port with RTP-port
                        return &((*it).second);
                    ++it;
                }
                return nullptr;
            }
            
            inline unsigned long getNumberOfRemotes() const
            {
                return remoteAgents.size();
            }
            
        private:
            std::map<std::string, SIPUserAgent> remoteAgents;
        };
    }
}
#endif	/* SIPUSERAGENT_H */

