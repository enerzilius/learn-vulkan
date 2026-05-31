#include <stdio.h>
#include <unordered_map>
#include <vector>

std::unordered_map<char, std::vector<int>> cores = {
    {'p', {0, 0, 1}},
    {'m', {1, 1, 10}},
    // [...]
};

int controle = 0;
int coresLidas[3];

void loop() {
  while (controle < 3) { // enquanto pra sempre
    char cor;

    cor = Serial.read();

    coresLidas[controle] = cores[cor][controle];

    printf("Cor %d: %c\n", controle, cor);
    controle++;
    // delay
  }

  int conta = (coresLidas[0] * 10 + coresLidas[1]) * coresLidas[2];
  printf("Conta = %d\n", conta);
}
