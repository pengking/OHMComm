/* 
 * File:   AudioProcessor.h
 * Author: daniel
 *
 * Created on March 29, 2015, 12:58 PM
 */

#ifndef AUDIO_PROCESSOR_H
#define	AUDIO_PROCESSOR_H

#include "configuration.h"
#include "RtAudio.h"
/*!
 * Abstract supertype for all classes used for intermediate handling of the input/output stream.
 * 
 * An implementation of this class may be used to encode/decode, filter or compress/decompress the input- and output-streams.
 * 
 * Processors can be chained, i.e. an output stream can be filtered -> encoded -> compressed.
 * The appertaining input-stream then will be decompressed -> decoded.
 */
class AudioProcessor
{
public:
    
    AudioProcessor();
    
    AudioProcessor(const AudioProcessor& orig);
    
    virtual ~AudioProcessor();
    
    /*!
     * Intermediate method processing the input/output stream and passing it onto the underlying processor.
     * 
     * For more information, see RtAudioCallback in RtAudio.h
     * 
     * \param outputBuffer For input (or duplex) streams,
     *      the outputBuffer of the underlying RtAudioCallback should be read, processed and written into this buffer
     * 
     * \param inputBuffer For output (or duplex) streams,
     *      the contents of this buffer are read, processed and written to the underlying inputBuffer.
     *      Like the outputBuffer, this holds \c nFrames frames of data
     * 
     * \param nFrames The number of input or output frames stored in the buffer
     * 
     * \param streamTime The number of seconds that have elapsed since the stream was started.
     * 
     * \param status If non-zero, this argument indicates a data overflow or underflow condition for the stream. 
     *      The particular condition can be determined by comparison with the RtAudioStreamStatus flags.
     * 
     * \param userData A pointer to optional user-data
     * 
     * This method returns zero to continue normal function. 
     * It returns a value of one to stop the stream immediately and a value of two to drain the buffer and stop the stream.
     */
    virtual int process( void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);
    
    /*!
     * Overwrite this method, if this AudioProcessor needs configuration
     * 
     * For the style of configuration, see OhmComm.cpp
     */
    virtual void configure();
    
    /*!
     * Sets the AudioProcessor next in chain
     */
    void setNextInChain(AudioProcessor *nextInChain);
    
    /*!
     * Gets the AudioProcessor next in chain
     */
    AudioProcessor *getNextInChain();
    
protected:
    AudioProcessor *nextInChain;
};

#endif	/* AUDIO_PROCESSOR_H */

