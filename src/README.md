# Explicação sobre a classe PCM e o arquivo main.cpp

## pcm.h

### Noções gerais

Neste arquivo, temos todas as definições necessárias para a utilização e input de um áudio do tipo PCM, tais como a estrutura de seu cabeçalho e a definição dos tipos de variáveis.

Eu escolhi utilizar a biblioteca stdint.h pois ela garante o tamanho de cada tipo de variável, tais como o uint8\_t, que garante que uma variável com esse tipo ocupará 8 bits de memória, e será unsigned (apenas números positivos), e assim por diante.

As definições de tipos feitas no começo do arquivo foram feitas para auxiliar na criação dos cabeçalhos, com os nomes dos tipos de variáveis que sejam mais fácil de memorizar do que algo como uint32\_t.

### Cabeçalhos

Os cabeçalhos são os primeiros bytes que ocupam um arquivo de um certo formato. E para cada byte, sua posição dentro do arquivo corresponde a um certo atributo do próprio arquivo, como por exemplo, no caso do PCM, temos que os 4 primeiros bytes correspondem a um identificador de um bloco, chamado de bloco RIFF, e que eles devem corresponder aos caracteres 'R', 'I', 'F', 'F', nesta ordem, precisamente.

Em seguida, temos as definições de todos os cabeçalhos que pertencem a um arquivo deste formato, que podem ser vistas na imagem a seguir.

![alt text](http://soundfile.sapp.org/doc/WaveFormat/wav-sound-format.gif)

### Classe PCM

Logo antes da classe PCM, criei uma exception básica para controlar erros ao instanciar um objeto da classe PCM.

A sintaxe para realizar herança em C++ é um pouco diferente da de Java, e ela segue um padrão da seguinte forma.

Em java:
```java
public class MinhaClasse extends ClassePai {
	// implementação da classe
}
```

Em C++:
```cpp
class MinhaClasse : public ClassePai {
	// implementação da classe
}
```

Logo depois, eu defini os métodos e atributos para a classe PCM, que serão implementadas no arquivo pcm.cpp. Em C++, diferente de Java, é possível (e preferível) separar a implementação dos métodos de uma certa classe da sua definição. Isto pode ser pensado como uma "interface" em Java, onde seus métodos são implementados em outra classe. Só que em C++, essa implementação é da **mesma classe**, porém, em arquivos diferentes.

### Pre-processador ifndef \_PCM\_H

Isso é feito por um motivo bastante simples e compreensível. Ao fazer um import de um arquivo em C++, diferente de Java (que importamos classes ou pacotes), nós importamos arquivos .h usando o pre-processador #include. Mas, caso aconteça de incluirmos o mesmo arquivo .h mais de uma vez (e que não é difícil de acontecer), temos que nos certificar de que não iremos definir a mesma coisa duas vezes, que pode causar um erro de compilação do código. Ai que entra o pré-processador #ifndef. O que ele faz é basicamente perguntar ao compilador se ja definimos uma variável chamada \_PCM\_H (esse nome é apenas uma convenção usada entre programadores) usando o pré-processador #define. Se não tivermos definido ele (importamos o arquivo pela primeira vez), definimos tudo que a gente precisa para utilizar a nossa biblioteca, incluindo o próprio \_PCM\_H, e caso contrário (importamos pela segunda vez ou mais), não iremos definir mais nada para evitar o erro de compilação.
