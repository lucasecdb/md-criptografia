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

### Pré-processador \#ifndef \_PCM\_H

Isso é feito por um motivo bastante simples e compreensível. Ao fazer um import de um arquivo em C++, diferente de Java (que importamos classes ou pacotes), nós importamos arquivos .h usando o pre-processador #include. Mas, caso aconteça de incluirmos o mesmo arquivo .h mais de uma vez (e que não é difícil de acontecer), temos que nos certificar de que não iremos definir a mesma coisa duas vezes, que pode causar um erro de compilação do código. Ai que entra o pré-processador #ifndef. O que ele faz é basicamente perguntar ao compilador se ja definimos uma variável chamada \_PCM\_H (esse nome é apenas uma convenção usada entre programadores) usando o pré-processador #define. Se não tivermos definido ele (importamos o arquivo pela primeira vez), definimos tudo que a gente precisa para utilizar a nossa biblioteca, incluindo o próprio \_PCM\_H, e caso contrário (importamos pela segunda vez ou mais), não iremos definir mais nada para evitar o erro de compilação.

## pcm.cpp

Neste arquivo temos a implementação de todos os métodos que foram definidos no arquivo pcm.h. É este arquivo que iremos compilar para depois, quando realizarmos a linkagem, juntar com o main.cpp.

Primeiramente, incluimos todas as nossas definiçoes do arquivo pcm.h fazendo o import com o pré-processador #include. Neste caso, usamos as aspas em "pcm.h" pois ele é um arquivo que está no mesmo diretório que o arquivo que o está importando.

Em seguida, definimos o código que vai ser executado em cada um dos métodos que nós tinhamos apenas escrito a assinatura em pcm.h.

### PCM\_exception

As duas primeiras funçoes que definimos nesse arquivo são o destrutor e what de PCM\_exception. O destrutor é o oposto de um construtor, ele executa antes do objeto ser retirado da memória e, geralmente, faz um clean-up de memória alocada (se tiver ocorrido alguma).

A funcão what faz o mesmo que a função getMessage() da classe Exception de Java, ela retorna uma string constante da descrição do erro. A keyword const e a expressão throw() após a assinatura e antes do corpo do método significa que essa função vai ser constante e que ela pode lançar exceptions também, que seria ao equivalente de termos a keyword final e um throws em um método de java.

### bytecmp

Essa função é criada nesse arquivo porque não é necessário ela ser disponibilizada junto da biblioteca, visto que ela é apenas uma função de utilidade interna. Ela compara os n primeiros bytes de duas string e retorna se elas são iguais ou não.

### PCM

Aqui definimos o corpo do contrutor, destrutor, e gets.

No construtor, dizemos que, pela sua assinatura, é possível lançar uma exception do tipo PCM\_exception, que será em caso não termos um formato de arquivo válido ou não conseguirmos abrir o arquivo que foi passado. Primeiro abrimos um arquivo cujo nome foi passado como string no construtor e verificamos se conseguirmos abrir ele (olhando se seu valor for NULL). Caso tenhamos conseguido abrir o arquivo, nós iremos ler os primeiros 36 bytes e iremos colocar esse valor dentro do atributo wav\_hdr desse objeto. Logo em seguida, verificamos o formato, usando a função bytecmp que foi definida anteriormente e, em caso de formato inválido, lançaremos uma exception. Depois, iremos continuar lendo no arquivo até acharmos um cabeçalho que seu identificador seja "data". Agora, iremos alocar memória dinamicamente para evitar corrupção de dados e guardar essa memória dentro do atributo data desse objeto. Por fim, iremos ler do arquivo os dados do arquivo e iremos fechá-lo.

As últimas funções são os gets dos atributos do objeto PCM.
