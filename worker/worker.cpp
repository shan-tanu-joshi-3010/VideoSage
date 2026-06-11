#include <iostream>
#include <rdkafkacpp.h>

int main()
{
    std::string brokers = "localhost:9092";
    std::string topic = "video-jobs";

    std::string errstr;

    auto conf =
        RdKafka::Conf::create(
            RdKafka::Conf::CONF_GLOBAL);

    conf->set("bootstrap.servers",
              brokers,
              errstr);

    conf->set("group.id",
              "video-worker-group",
              errstr);

    auto consumer =
        RdKafka::KafkaConsumer::create(
            conf,
            errstr);

    if (!consumer)
    {
        std::cerr
            << "Consumer creation failed: "
            << errstr
            << std::endl;

        return 1;
    }

    consumer->subscribe({topic});

    std::cout
        << "Worker started..."
        << std::endl;

    while (true)
    {
        auto msg =
            consumer->consume(1000);

        switch (msg->err())
        {
        case RdKafka::ERR_NO_ERROR:

            std::cout
                << "Received: "
                << static_cast<const char*>(
                       msg->payload())
                << std::endl;

            break;

        case RdKafka::ERR__TIMED_OUT:
            break;

        default:

            std::cerr
                << msg->errstr()
                << std::endl;
        }

        delete msg;
    }

    consumer->close();

    delete consumer;
    delete conf;
}