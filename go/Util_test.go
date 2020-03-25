/*
 * @Author: Bingo
 * @Date: 2020-03-25 12:27:21
 * @LastEditTime: 2020-03-25 14:18:02
 * @Description:
 * @FilePath: /pcm2wav/go/Util_test.go
 */
package util

import (
	"io/ioutil"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestPcm2Wav(t *testing.T) {
	var err error
	pcmName := "../test.pcm"
	resultWavName := "pcm2wav.wav"
	expectedWavName := "../expect.wav"

	err = Pcm2Wav(pcmName, resultWavName)
	if err != nil {
		assert.Error(t, err)
	}
	resultWav, err := ioutil.ReadFile(resultWavName)
	if err != nil {
		assert.Error(t, err)
	}

	expectedWav, err := ioutil.ReadFile(expectedWavName)
	if err != nil {
		assert.Error(t, err)
	}
	assert.Equal(t, expectedWav, resultWav, "Unexpected wav data")
}
