#ifndef CHAT_SERIAL_H
#define CHAT_SERIAL_H

/*!
 * \file chat_serial.h
 * \brief Common header for data serialization.
 * \author Vaccari Fabio
 * \version 0.1
 *
 * Defines common data types for chat clients and servers.
 *
 */

#include <QMetaType>
#include <QFlags>
#include <QDataStream>

/*!
 * \brief Helper container.
 *
 * The data stream uses Qt 5.2 version encoding, encodes packets in base 64.
 * A packet has no size specification and ends with a terminator character
 * \ref terminator() .
 * Data serialization is performed as follows:
 *
 *    - an header, consisting of
 *    	- mask \ref mask()
 *    	- version \ref version()
 *    - a body
 *    	- message type \ref MessageType
 *    	- data according to the message type
 *
 * details of the protocol:
 *
 *    - an empty message
 *    	-# mask \ref mask()
 *    	-# version \ref version()
 *    	-# \ref EmptyMessage
 *
 *    - login procedure: the client and the server have a working connection,
 *      the client is not logged, the user takes a nickname
 *    	- client sends login request
 *    		-# mask \ref mask()
 *    		-# version \ref version()
 *    		-# \ref LoginRequest
 *    		-# nickname
 *    	- server checks the nickname
 *    		- if no connected client is using the nickname
 *    			- server sends the nickname back to the client
 *    				-# mask \ref mask()
 *    				-# version \ref version()
 *    				-# \ref LoginResponse
 *    				-# the nickname
 *    			- server informs the client of the other connected clients
 *    				-# mask \ref mask()
 *    				-# version \ref version()
 *    				-# \ref ClientEntered
 *    				-# the nicknames of all connected clients
 *    			- server informs all connected clients of the new client
 *    				-# mask \ref mask()
 *    				-# version \ref version()
 *    				-# \ref ClientEntered
 *    				4. the nickname of the client
 *    			- the client is logged
 *    		- if a connected client holds the nickname, server sends
 *    		  no nickname
 *    			- server sends \ref LoginResponse to the client
 *    				-# mask \ref mask()
 *    				-# version \ref version()
 *    				-# \ref LoginResponse
 *    				-# empty string
 *    			- the client informs the user
 *
 *    - when a client disconnects, if it was logged, server informs
 *      the other connected clients
 *    	-# mask \ref mask()
 *    	-# version \ref version()
 *    	-# \ref ClientExited
 *    	-# nickname of the client
 *
 *    - sending a message: the client is connected with the
 *      server, the client is logged, the user instructs the client to send
 *      a message
 *    	- client sends to server
 *    		-# mask \ref mask()
 *    		-# version \ref version()
 *    		-# \ref TextMessage
 *    		-# his nickname
 *    		-# a list of receivers
 *    		-# the text
 *    	- server attaches a timestamp and sends to all
 *    	  the receivers in the list which are connected:
 *    		-# mask \ref mask()
 *    		-# version \ref version()
 *    		-# \ref TextMessage
 *    		-# timestamp
 *    		-# sender's nickname
 *    		-# the text
 *      note. There is no check if the receiver actually gets the message.
 */
struct Protocol {
    
    /*!
     * \brief Protocol version.
     * \return current version.
     */
    static
    qint64 version() { return 20140823; }
    
    /*!
     * \brief Protocol mask.
     * \return message mask.
     */
    static
    qint64 mask() { return 65537; }
    
    /*!
     * \brief Protocol terminator.
     * \return terminator character.
     */
    static
    char terminator() { return '~'; }
};

/*!
 * \brief initialize the output stream.
 * \param out output stream
 * \return output stream
 *
 * Convenience function for initializing the output stream.
 * Sets stream version, writes mask and version to the stream.
 * \see Protocol
 */
inline
QDataStream& setHeader(QDataStream &out) {
    out.setVersion(QDataStream::Qt_5_2);
    return out << Protocol::mask() << Protocol::version();
}

/*!
 * \brief Message type.
 *
 * List of implemented messages.
 */
enum MessageType {
    EmptyMessage, //!< as is.
    LoginRequest, //!< client's login request.
    LoginResponse, //!< server's response of a login request.
    ClientEntered, //!< list of clients which entered the chat.
    ClientExited, //!< list of clients which exited the chat.
    TextMessage //!< one to many text message.
};
Q_DECLARE_TYPEINFO(MessageType, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(MessageType)

/*!
 * \brief Serialize MessageType.
 * \param out output stream.
 * \param mt MessageType.
 * \return output stream.
 *
 * Write MessageType to a Qt data stream.
 */
inline
QDataStream& operator << (QDataStream &out, const MessageType &mt) {
    return out << static_cast<quint32>(mt);
}

/*!
 * \brief Deserialize MessageType.
 * \param in input stream.
 * \param mt data read.
 * \return  output stream.
 *
 * Read MessageType from a Qt data stream.
 */
inline
QDataStream& operator >> (QDataStream &in, MessageType &mt) {
    quint32 i;
    
    in >> i;
    mt = static_cast<MessageType>(i);
    return in;
}

/*!
 * \brief Register common metatypes to the Qt's type system.
 *
 * This function should be called once, before the first signal/slot
 * connection is estabilished.
 * \see qRegisterMetaType()
 */
inline
void registerSerialMetatypes() {
    qRegisterMetaType<MessageType>();
    qRegisterMetaTypeStreamOperators<MessageType>("MessageType");
}

#endif // CHAT_SERIAL_H
