#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <ctime>
#include <chrono>

#define POPULATION_SIZE 800 // размер популяции (число особей определяет разнообразие решений в каждом поколении)
#define GENERATIONS 100 // число поколений - определяет время жизни популяции, чем больше, тем больше шансов получить хорошее решение
#define MUTATION_RATE 0.1 // вероятность мутации - мутация позволяет выпрыгнуть из локального экстремума, но мутации не должны быть слишком частыми, чтобы решение сходилось. При частых мутациях решение будет слабо отличаться от случайного
#define BATTLE_SIZE 18 // число особей, участвующих в "битве" - имитация естественного отбора и борьбы за выживание
#define TOURNAMENT_SIZE 100 // число особей, участвующих в борьбе за выбор родителя

using namespace std;

mt19937 random_generator((random_device())());

bool generate_random_bool() noexcept {
    return static_cast<bool>(random_generator() & 1);
}

unsigned int random_unsigned_int() noexcept {
    return random_generator();
}

double random_double(double a, double b) noexcept {
    return uniform_real_distribution<double>(0, 1)(random_generator);
}

struct Item {
    int price;
    int weight;
};

// особь
struct Individual {
    vector<bool> DNA; // ДНК - это какие предметы взяты в рюкзак
    int fitness; // приспособленность к выживанию - это суммарная стоимость, если вес не превышает максимального, или 0 в противном случае

    Individual(int max_weight, const vector<Item>& items, const vector<bool>& individual_dna) noexcept :
        DNA(individual_dna),
        fitness(0) {
        int price = 0;
        int weight = 0;
        for (int i = 0; i < items.size(); ++i) {
            if (DNA[i]) {
                price += items[i].price;
                weight += items[i].weight;
            }
        }

        if (weight <= max_weight) {
            fitness = price;
        }
    }
};

Individual create_random_individual(int max_weight, const vector<Item>& items) noexcept {
    vector<bool> individual_dna(items.size());
    for (int i = 0; i < items.size(); ++i) {
        individual_dna[i] = generate_random_bool();
    }
    return Individual(max_weight, items, individual_dna);
}

/*
Естественный отбор, борьба за выживание.
Не используется в алгоритме, так как она снижает рождаемость.
Для использования нужно повысить рождаемость или оставлять родителей в новой популяции.
В первом случае нужно будет сильно увеличивать число поколений, и алгоритм будет работать дольше.
Во втором случае для сходимости придется ввести индекс поколения и убирать из популяции особей, которые прожили несколько поколений (например, 3).
*/
vector<Individual> natural_selection(const vector<Individual>& population) noexcept {
    vector<Individual> survivors; // выжившие после борьбы особи
    unordered_set<unsigned int> victims_indices; // уже рассмотренные особи, включая наиболее приспособленную
    auto next_victim = [&]() {
        unsigned int victim_index;
        do {
            victim_index = random_unsigned_int() % population.size();
        } while (victims_indices.contains(victim_index)); // не сражаемся с самим собой и не повторяемся в процессе борьбы - мертвые не встают
        victims_indices.insert(victim_index); // добавляем новую жертву
        return population[victim_index];
    };

    Individual best = next_victim();
    for (int i = 1; i < BATTLE_SIZE; ++i) {
        Individual victim = next_victim();
        if (best.fitness < victim.fitness) {
            best = victim; // обновляем "царя горы"
        }
    }

    survivors.push_back(best); // добавляем "царя горы" к выжившим особям
    for (int i = 0; i < population.size(); ++i) {
        if (!victims_indices.contains(i)) {
            survivors.push_back(population[i]);
        }
    }
    return survivors;
}

// борьба за выбор родителя
Individual tournament_selection(const vector<Individual>& population) noexcept {
    auto next_challenger = [&]() {
        return population[random_unsigned_int() % population.size()];
    };
    Individual best = next_challenger();
    for (int i = 1; i < TOURNAMENT_SIZE; ++i) {
        Individual challenger = next_challenger();
        if (best.fitness < challenger.fitness) {
            best = challenger;
        }
    }
    return best;
}

vector<bool> mutate_dna(vector<bool> dna) noexcept {
    for (int i = 0; i < dna.size(); ++i) {
        if (random_double(0, 1) < MUTATION_RATE) {
            dna[i] = !dna[i];
        }
    }
    return dna;
}

pair<Individual, Individual> crossover(int max_weight, const vector<Item> items, const Individual& mother, const Individual& father) noexcept {
    vector<bool> son_dna(items.size());
    vector<bool> daughter_dna(items.size());
    for (int i = 0; i < items.size(); ++i) {
        if (generate_random_bool()) {
            son_dna[i] = father.DNA[i];
            daughter_dna[i] = mother.DNA[i];
        } else {
            son_dna[i] = mother.DNA[i];
            daughter_dna[i] = father.DNA[i];
        }
    }
    return {
        Individual(max_weight, items, mutate_dna(son_dna)),
        Individual(max_weight, items, mutate_dna(daughter_dna))
    };
}

int solve(int max_weight, const vector<Item>& items) noexcept {
    vector<Individual> population;
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        population.push_back(create_random_individual(max_weight, items));
    }

    for (int generation = 0; generation < GENERATIONS; ++generation) {
        vector<Individual> new_population;
        while (new_population.size() < POPULATION_SIZE) {
            Individual mother = tournament_selection(population);
            Individual father = tournament_selection(population);

            auto [son, daughter] = crossover(max_weight, items, mother, father);
            new_population.push_back(son);
            new_population.push_back(daughter);
        }
        population = move(new_population);
    }

    int best_price = 0;
    for (const auto& individual : population) {
        best_price = max(best_price, individual.fitness);
    }
    return best_price;
}

vector<Item> manual_input(int& n, int& max_weight) noexcept {
    cout << "Input number of items: ";
    cin >> n;
    cout << "Input max backpack weight: ";
    cin >> max_weight;
    cout << "Input n pairs: price - weight\n";
    vector<Item> items(n);
    for (int i = 0; i < n; ++i) {
        cin >> items[i].price >> items[i].weight;
    }
    return items;
}

vector<Item> file_input(int& n, int& max_weight, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл '" << filename << "'.\n";
        return {};
    }

    file >> n >> max_weight;
    vector<Item> items(n);
    for (int i = 0; i < n; ++i) {
        file >> items[i].price >> items[i].weight;
    }
    return items;
}

vector<Item> input_data(int& n, int& max_weight) {
    string filename;
    cout << "Enter input filename or press Enter to manually input data: ";
    getline(cin, filename);
    if (filename.empty()) {
        return manual_input(n, max_weight);
    }
    return file_input(n, max_weight, filename);
}

int main() {
    int n, max_weight;
    vector<Item> items = input_data(n, max_weight);
    auto start = chrono::high_resolution_clock::now();
    int best_price = solve(max_weight, items);
    auto end = chrono::high_resolution_clock::now();
    auto time_spent = end - start;
    cout << "Maximum price that can be taken: " << best_price << '\n';
    cout << "Time spent:\n";
    cout << "t = " << time_spent.count() << " nanoseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::milliseconds>(time_spent).count() << " milliseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::seconds>(time_spent).count() << " seconds\n";
#if defined(NDEBUG)
    std::cout << "Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
#endif
    return 0;
}