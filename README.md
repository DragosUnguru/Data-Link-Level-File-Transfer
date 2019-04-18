# BRIEF INTRODUCTION:
This program represents a reliable transmission of
data frames between two nodes connected by a physical layer,
a representation of the data link model.

The physical layer is emulated using two separate processes
that play the role of the sender and the receiver.

It's implemented using a gliding window method, flooding
the physical layer with the maximum number of frames, and
continuing to send frames for every acknowledgement.

The simplest approach was taken. If the receiver has
a corrupted frame, validated with a checksum (xor for every
byte of the frame's payload), he discards the frame, sending
back a NACK indicating the next in-order expected frame number.

# EOF SIGNAL:
We take advantage of the fact that ACKs will never get
lost or corrupted, so we rely on this for sending the terminating
frame. The receiver stops when the buffer is complete and ready
to be flushed to the file.

# DATA STRUCTURE:
The receiver doesn't let any valid frame to waste. Even though
the frame isn't the one he was expecting, he buffers it in a 
(kind of) priority queue.

This queue orders all the frames gotten so far by their
sequence number. It doesn't allow for any duplicates! It's
implemented in such a way that every "push" action will check
for a duplicate and position the new frame accordingly.

This approach gives us a reasonable query time for wanted
frames and for getting the next expected sequence but the biggest
benefit is the ease gained from a implementation point of view.

# TIMEOUT:
If the receiver timed out, send a window of NACKs
for the first chunk of consecutive missing
frames. This way, we force the sender to send
another window of frames. The sender (send.c) will handle these
nack'd packages and send them accordingly, keeping
the connection as busy as possible.

This way, we can safely minimize the TIMEOUT, keeping
in mind that as long as the acks are received instantly,
we have RTT = 1 * delay. We don't have to worry about adding
an error margin (epsilon) as we can rely on the receiver (recv.c)
that will keep the wire full by sending chunks of needed frames.
