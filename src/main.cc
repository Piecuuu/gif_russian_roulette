#include "gif_extractor.h"
#include <inttypes.h>
#include <ctime>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <format>

#define DISCORD_API std::string("https://discord.com/api/v9")
#define DISCORD_CHANNELS_ENDPOINT DISCORD_API + "/channels/"
#define DISCORD_MESSAGE_ENDPOINT "/messages"
#define USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/121.0"

struct curl_slist *list = NULL;

std::string get_random_gif(std::vector<std::string> links) {
  int random_value = rand() % links.size();
  std::string random_gif = links[random_value];
  return random_gif;
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

nlohmann::json get_channel(std::string id) {
  CURL* curl = curl_easy_init();
  CURLcode res;
  std::string data;

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, std::format("{}{}", DISCORD_CHANNELS_ENDPOINT, id).c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      throw std::runtime_error("Failed to get the channel");
    }
    curl_easy_cleanup(curl);
  } else {
    throw std::runtime_error("Couldn't initiate CURL");
  }
  return nlohmann::json::parse(data);
}

void send_to_channel(std::string channel_id, std::string content) {
  CURL* curl = curl_easy_init();
  CURLcode res;
  std::string data;
  std::string json_data = std::format(R"({{"content":"{}"}})", content);

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, std::format("{}{}{}", DISCORD_CHANNELS_ENDPOINT, channel_id, DISCORD_MESSAGE_ENDPOINT).c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      throw std::runtime_error("Failed to get the channel");
    }
    curl_easy_cleanup(curl);
  }
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("gif_russian_roulette");
  program.add_argument("-t", "--token")
    .help("Discord token to play the game")
    .required();
  program.add_argument("-c", "--channel")
    .help("The channel ID to play the game on")
    .required();
  program.add_argument("--no-spoiler")
    .help("If on, the bot will not send the message as a spoiler")
    .flag();
  program.add_argument("--allow-send-on-nsfw-channel-dangerous")
    .help("If on, the bot will send gifs on a non-NSFW channel (DANGEROUS)")
    .flag();

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  std::string token = program.get<std::string>("--token");
  std::string channel_id = program.get<std::string>("--channel");

  list = curl_slist_append(list, std::format("authorization: {}", token).c_str());
  list = curl_slist_append(list, "Accept: */*");
  list = curl_slist_append(list, "Content-type: application/json");

  nlohmann::json channel = get_channel(channel_id);
  if(!channel["nsfw"] && program["--allow-send-on-nsfw-channel-dangerous"] == false) {
    throw std::runtime_error("Cannot play on a non-NSFW channel. Use --allow-send-on-nsfw-channel-dangerous to bypass (DANGEROUS)");
    return 1;
  }

  std::string data = GifExtractor::get_data_by_token(token);
  std::string json_data = GifExtractor::protobuf_to_json(data);
  std::vector<std::string> links = GifExtractor::parse_json(json_data);

  srand(static_cast<uint32_t>(time(NULL)));

  std::cout << "Play? ";
  getchar();

  randomgif:
    std::string random_gif = get_random_gif(links);
    std::string random_gif_no_spoiler = random_gif;
    if(program["--no-spoiler"] == false) {
      random_gif = std::format("||{}||", random_gif);
    }

    send_to_channel(channel_id, random_gif);

    std::cout << random_gif_no_spoiler << "\nPlay again? ";
    getchar();
    goto randomgif;
}
