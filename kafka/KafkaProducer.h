#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <string>
#include <memory>
#include <rdkafkacpp.h>

class KafkaProducer
{
public:
    KafkaProducer(const std::string& brokers,
                  const std::string& topic);

    ~KafkaProducer();

    bool sendJob(const std::string& videoId,
                 const std::string& videoPath);

private:
    std::string topicName;

    RdKafka::Producer* producer;
    RdKafka::Topic* topic;
};

#endif