using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ArduinoRemoteControl
{
    /// <summary>
    /// Class to extract message from a text feed (like serial port).
    /// 
    /// Feed all data from the feed to the MessageBuilder by using the Add() method.
    /// 
    /// </summary>
    public class MessageBuilder
    {
        /// <summary>
        /// Message from which more characters are to be expected from the feed.
        /// </summary>
        private string partlyMessage;

        /// <summary>
        /// Marker that marks the mark the end of a message.
        /// </summary>
        public char MessageEndMarker { get; private set; }

        /// <summary>
        /// Create a MessageBuilder instance.
        /// </summary>
        /// Marker that is used to find the end of a message 
        /// when trying to find messages in the buffered data.
        /// </param>
        public MessageBuilder()
        {
            MessageEndMarker = '\n';
            partlyMessage = null;
        }

        /// <summary>
        /// Create a MessageBuilder instance.
        /// </summary>
        /// </param>
        /// <param name="messageEndMarker">
        /// Marker that is used to find the end of a message 
        /// when trying to find messages in the buffered data.
        /// </param>
        public MessageBuilder(char messageEndMarker)
        {
            MessageEndMarker = messageEndMarker;
            partlyMessage = null;
        }

        /// <summary>
        /// Feeds data containing (possible) message to the MessageBuilder.
        /// After using Add, if message is complete the message is returned else an empty string is returned
        /// 
        /// Its possible that an incomplete message is contained in the data.
        /// </summary>
        /// <param name="data">
        /// data from the feed containing possible messages.
        /// </param>
        public string Add(string data)
        {
            if (data == null)
            {
                throw new ArgumentNullException("data");
            }

            string message;
            if (partlyMessage != null)
            {
                message = partlyMessage;
                partlyMessage = null;
            }
            else
            {
                message = "";
            }

            foreach (char character in data)
            {
                if (character != MessageEndMarker)
                {
                    message += character;
                }
                else
                {
                    return message;
                }
            }
            partlyMessage = message;
            return "";
        }

        /// <summary>
        /// Clear all buffered message data
        /// </summary>
        public void Clear()
        {
            partlyMessage = null;
        }
    }
}
