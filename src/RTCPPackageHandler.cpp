/* 
 * File:   RTCPPackageHandler.cpp
 * Author: daniel
 * 
 * Created on June 16, 2015, 5:47 PM
 */

#include "RTCPPackageHandler.h"

RTCPPackageHandler::RTCPPackageHandler()
{
    //maximal SR size: 8 (header) + 20 (sender-info) + 31 (5 bit RC count) * 24 (reception report) = 772
    //maximal RR size: 8 (header) + 31 (5 bit RC count) * 24 (reception report) = 752
    //maximal SDES size: 8 (header) + 31 (5 bit SDES count) * (1 (SDES type) + 1 (SDES length) + 255 (max 255 characters)) = 7975
    //maximal BYE size: 8 (header) + 1 (length) + 255 (max 255 characters) = 264
    rtcpPackageBuffer = new char[8000];
}

RTCPPackageHandler::~RTCPPackageHandler()
{
    free(rtcpPackageBuffer);
}


void* RTCPPackageHandler::createSenderReportPackage(RTCPHeader& header, SenderInformation& senderInfo, std::vector<ReceptionReport> reports)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_SENDER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    header.length = calculateLengthField(RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);
    
    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE, &senderInfo, RTCP_SENDER_INFO_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createReceiverReportPackage(RTCPHeader& header, std::vector<ReceptionReport> reports)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_RECEIVER_REPORT;
    header.receptionReportOrSourceCount = reports.size();
    header.length = calculateLengthField(RTCP_HEADER_SIZE + reports.size() * RTCP_RECEPTION_REPORT_SIZE);
    
    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    for(unsigned int i = 0; i < reports.size(); i++)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + i * RTCP_RECEPTION_REPORT_SIZE, &reports[i], RTCP_RECEPTION_REPORT_SIZE);
    }
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createSourceDescriptionPackage(RTCPHeader& header, std::vector<SourceDescription> descriptions)
{
    //adjust header
    header.packageType = RTCP_PACKAGE_SOURCE_DESCRIPTION;
    header.receptionReportOrSourceCount = descriptions.size();

    uint16_t offset = RTCP_HEADER_SIZE;
    for(unsigned int i = 0; i < descriptions.size(); i++)
    {
        //set type
        memcpy(rtcpPackageBuffer + offset, &descriptions[i].type, 1);
        offset++;
        //set length
        uint8_t size = descriptions[i].value.size();
        memcpy(rtcpPackageBuffer + offset, &size, 1);
        offset++;
        //set value
        memcpy(rtcpPackageBuffer + offset, descriptions[i].value.c_str(), size);
        offset+=size;
    }
    //now offset is the same as the payload length
    //padding to the next multiple of 4 bytes
    uint8_t padding = 4 - ((1 + offset) % 4);
    header.length = calculateLengthField(offset);
    header.padding = padding != 0;
    
     if(padding != 0)
    {
        //apply padding - fill with number of padded bytes
        memset(rtcpPackageBuffer + offset, padding, padding);
    }
    
    //we need to copy the header last, because of the length- and padding-fields
    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createByePackage(RTCPHeader& header, std::string byeMessage)
{
    header.packageType = RTCP_PACKAGE_GOODBYE;
    uint8_t length = byeMessage.size();
    //padding to the next multiple of 4 bytes
    uint8_t padding = 4 - ((1 + length) % 4);
    header.length = calculateLengthField(RTCP_HEADER_SIZE + 1 + length);
    header.padding = padding != 0;
    
    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE, &length, 1);
    if(length > 0)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + 1, byeMessage.c_str(), length);
    }
    if(padding != 0)
    {
        //apply padding - fill with number of padded bytes
        memset(rtcpPackageBuffer + RTCP_HEADER_SIZE + 1 + length, padding, padding);
    }
    return rtcpPackageBuffer;
}

void* RTCPPackageHandler::createApplicationDefinedPackage(RTCPHeader& header, ApplicationDefined& appDefined)
{
    header.packageType = RTCP_PACKAGE_APPLICATION_DEFINED;
    header.length = calculateLengthField(RTCP_HEADER_SIZE + 4 + appDefined.dataLength);
    //application defined data must be already padded to 32bit
    header.padding = 0;
    header.receptionReportOrSourceCount = appDefined.subType;
    
    memcpy(rtcpPackageBuffer, &header, RTCP_HEADER_SIZE);
    memcpy(rtcpPackageBuffer+ RTCP_HEADER_SIZE, appDefined.name, 4);
    if(appDefined.dataLength > 0)
    {
        memcpy(rtcpPackageBuffer + RTCP_HEADER_SIZE + 4, appDefined.data, appDefined.dataLength);
    }
    return rtcpPackageBuffer;
}


std::vector<ReceptionReport> RTCPPackageHandler::readSenderReport(void* senderReportPackage, uint16_t packageLength, RTCPHeader& header, SenderInformation& senderInfo)
{
    memcpy(rtcpPackageBuffer, senderReportPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackageBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    SenderInformation *readInfo = (SenderInformation *)rtcpPackageBuffer + RTCP_HEADER_SIZE;
    //copy sender-info to out-parameter
    senderInfo = *readInfo;
    
    std::vector<ReceptionReport> reports(header.receptionReportOrSourceCount);
    for(unsigned int i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        ReceptionReport *readReport = (ReceptionReport *)rtcpPackageBuffer + RTCP_HEADER_SIZE + RTCP_SENDER_INFO_SIZE + i * RTCP_RECEPTION_REPORT_SIZE;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<ReceptionReport> RTCPPackageHandler::readReceiverReport(void* receiverReportPackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(rtcpPackageBuffer, receiverReportPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackageBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    std::vector<ReceptionReport> reports(header.receptionReportOrSourceCount);
    for(unsigned int i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        ReceptionReport *readReport = (ReceptionReport *)rtcpPackageBuffer + RTCP_HEADER_SIZE + i * RTCP_RECEPTION_REPORT_SIZE;
        reports[i] = *readReport;
    }
    return reports;
}

std::vector<SourceDescription> RTCPPackageHandler::readSourceDescription(void* sourceDescriptionPackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(rtcpPackageBuffer, sourceDescriptionPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackageBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    std::vector<SourceDescription> descriptions(header.receptionReportOrSourceCount);
    uint16_t offset = RTCP_HEADER_SIZE;
    for(uint8_t i = 0; i < header.receptionReportOrSourceCount; i++)
    {
        RTCPSourceDescriptionType *type = (RTCPSourceDescriptionType*)rtcpPackageBuffer + offset;
        offset++;
        uint8_t *valueLength = (uint8_t *)rtcpPackageBuffer + offset;
        offset++;
        char *value = rtcpPackageBuffer + offset;
        descriptions[i].type = *type;
        descriptions[i].value = std::string(value, *valueLength);
        offset += *valueLength;
    }
    return descriptions;
}


std::string RTCPPackageHandler::readByeMessage(void* byePackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(rtcpPackageBuffer, byePackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackageBuffer;
    //copy header to out-parameter
    header = *readHeader;
    uint8_t length = *(rtcpPackageBuffer + RTCP_HEADER_SIZE);
    
    const char *byeMessage = (const char*)rtcpPackageBuffer + RTCP_HEADER_SIZE + 1;
    return std::string(byeMessage, length);
}

ApplicationDefined RTCPPackageHandler::readApplicationDefinedMessage(void* appDefinedPackage, uint16_t packageLength, RTCPHeader& header)
{
    memcpy(rtcpPackageBuffer, appDefinedPackage, packageLength);
    
    RTCPHeader *readHeader = (RTCPHeader *)rtcpPackageBuffer;
    //copy header to out-parameter
    header = *readHeader;
    
    char name[4];
    memcpy(name, rtcpPackageBuffer + RTCP_HEADER_SIZE, 4);
    //length of whole package - 3 lines (2 for header, 1 for name) -> convert to bytes
    uint16_t dataLength = (readHeader->length - 3) * 4;
    char * data = rtcpPackageBuffer + RTCP_HEADER_SIZE + 4;
    
    ApplicationDefined result(name, dataLength, data, readHeader->receptionReportOrSourceCount);
    return result;
}


uint8_t RTCPPackageHandler::calculateLengthField(uint16_t length)
{
    //4 byte = 32 bit
    //we need to always round up, so we add 3:
    //e.g. 16 / 4 = 4 <-> (16+3) / 4 = 4
    //17 / 4 = 4 => (17+3) / 4 = 5!
    return ((length+3) / 4) - 1;
}
