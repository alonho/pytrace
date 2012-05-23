#ifndef CODEC_H
#define CODEC_H

unsigned long encode(const unsigned char *source,
		     unsigned long size,
		     unsigned char *dest);

unsigned long decode(const unsigned char *source,
		     unsigned long size, 
		     unsigned char *dest);

#endif /* codec */
