/*
 * @Author: Bingo
 * @Date: 2020-03-25 11:42:41
 * @LastEditTime: 2020-03-25 14:20:45
 * @Description:
 * @FilePath: /pcm2wav/go/Util.go
 */
package util

import (
	"encoding/binary"
	"errors"
	"os"
)

type wavFormatChunk struct {
	chunkID       [4]byte /* {'f', 'm', 't', ' '} */
	chunkSize     uint32
	audioFormat   uint16
	numChannels   uint16
	sampleRate    uint32
	byteRate      uint32
	blockAlign    uint16
	bitsPerSample uint16
}

type wavDataChunk struct {
	chunkID   [4]byte /* {'d', 'a', 't', 'a'}  */
	chunkSize uint32
	data      []byte
}

func Pcm2Wav(pcmName string, wavName string) error {
	pcm, err := os.Open(pcmName)
	if err != nil {
		return errors.New("failed to open pcm file.")
	}
	defer pcm.Close()
	wav, err := os.OpenFile(wavName, os.O_WRONLY|os.O_TRUNC|os.O_CREATE, 0666)
	if err != nil {
		return errors.New("failed to create wav file.")
	}
	defer wav.Close()
	buf := make([]byte, 1024)

	pcmSize, _ := pcm.Seek(0, os.SEEK_END)
	pcm.Seek(0, os.SEEK_SET)

	binary.Write(wav, binary.BigEndian, []byte("RIFF"))
	binary.Write(wav, binary.LittleEndian, uint32(36+pcmSize))
	binary.Write(wav, binary.BigEndian, []byte("WAVE"))

	formatChunk := wavFormatChunk{
		chunkID:       [4]byte{'f', 'm', 't', ' '},
		chunkSize:     16,
		audioFormat:   1,
		numChannels:   1,
		sampleRate:    16000,
		byteRate:      16000 * 1 * (16 >> 3), //sampleRate * numChannels * (bitsPerSample >> 3),
		blockAlign:    1 * (16 >> 3),         //numChannels * (bitsPerSample >> 3),
		bitsPerSample: 16,
	}
	binary.Write(wav, binary.BigEndian, formatChunk.chunkID)
	binary.Write(wav, binary.LittleEndian, formatChunk.chunkSize)
	binary.Write(wav, binary.LittleEndian, formatChunk.audioFormat)
	binary.Write(wav, binary.LittleEndian, formatChunk.numChannels)
	binary.Write(wav, binary.LittleEndian, formatChunk.sampleRate)
	binary.Write(wav, binary.LittleEndian, formatChunk.byteRate)
	binary.Write(wav, binary.LittleEndian, formatChunk.blockAlign)
	binary.Write(wav, binary.LittleEndian, formatChunk.bitsPerSample)

	dataChunk := wavDataChunk{
		chunkID:   [4]byte{'d', 'a', 't', 'a'},
		chunkSize: uint32(pcmSize),
	}
	binary.Write(wav, binary.BigEndian, dataChunk.chunkID)
	binary.Write(wav, binary.LittleEndian, dataChunk.chunkSize)
	for {
		len, err := pcm.Read(buf)
		binary.Write(wav, binary.LittleEndian, buf[:len])
		if err != nil {
			break
		}
	}
	return nil
}
