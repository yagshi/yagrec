# yagrec
ALSA recorder

## abstract (概要)
Record sound using ALSA only when sound exists.

Alexa のパチもんみたいなのを作るときに結構悩むのが
「**音があるときだけ録音**する」方法。sox の rec コマンドは
いいところまでできるんだけど、冒頭部分が欠ける（そして
音声認識でコケる）。

このツールは音声があるレベルを超えた瞬間から少し遡って
録音できます。


## usage (使い方)

yagrec [-D dev] [-m margin_in_second] [-t threshold(0-32767)]

  - dev         device name (e.g. hw:1,0)
  - margin      recording margin BEFORE sound exisits  
            音がなった瞬間から遡って録音する秒数
  - threshold   threshold level of sound


  - A wave format data is output through **STDOUT**.
  - 出力は wav ファイル形式で **標準出力** から出てきます。
