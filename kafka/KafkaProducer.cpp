#include "KafkaProducer.h"

#include <iostream>
#include <crow/json.h>

KafkaProducer::KafkaProducer(const std::string& brokers,
                             const std::string& topicName)
    : producer(nullptr),
      topic(nullptr),
      topicName(topicName)
{
    std::string errstr;

    RdKafka::Conf* conf =
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    conf->set("bootstrap.servers",
              brokers,
              errstr);

    producer =
        RdKafka::Producer::create(conf,
                                  errstr);

    if (!producer)
    {
        throw std::runtime_error(
            "Failed to create producer: "
            + errstr);
    }

    topic =
        RdKafka::Topic::create(producer,
                               topicName,
                               nullptr,
                               errstr);

    if (!topic)
    {
        throw std::runtime_error(
            "Failed to create topic: "
            + errstr);
    }

    delete conf;
}

KafkaProducer::~KafkaProducer()
{
    producer->flush(5000);

    delete topic;
    delete producer;
}

bool KafkaProducer::sendJob(const std::string& videoId,
                            const std::string& videoPath)
{
    crow::json::wvalue job;

    job["video_id"] = videoId;
    job["video_path"] = videoPath;

    std::string payload = job.dump();

    RdKafka::ErrorCode err =
        producer->produce(
            topic,
            RdKafka::Topic::PARTITION_UA,
            RdKafka::Producer::RK_MSG_COPY,
            const_cast<char*>(payload.c_str()),
            payload.size(),
            nullptr,     // key
            0,           // key length
            nullptr      // message opaque
        );

    producer->poll(0);

    if (err != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Failed to publish message: "
                  << RdKafka::err2str(err)
                  << std::endl;
        return false;
    }

    return true;
}