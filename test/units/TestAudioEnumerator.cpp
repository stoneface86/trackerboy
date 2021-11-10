
#include "units/TestAudioEnumerator.hpp"

#include "audio/AudioEnumerator.hpp"


TestAudioEnumerator::TestAudioEnumerator() {

}

void TestAudioEnumerator::deviceSerialization() {
    AudioEnumerator enumerator;

    QVariant badData = 1;

    for (int backend = 0; backend < enumerator.backends(); ++backend) {
        enumerator.populate(backend);

        auto const devices = enumerator.devices(backend);
        for (int device = 0; device < devices; ++device) {

            auto data = enumerator.serializeDevice(backend, device);
            auto deviceCheck = enumerator.deserializeDevice(backend, data);

            QVERIFY(device == deviceCheck);


        }

        // check that invalid data results in -1
        QVERIFY(enumerator.deserializeDevice(backend, badData) == -1);

        // check for invalid index
        QVERIFY(enumerator.serializeDevice(backend, -1).isNull());

    }

    QVERIFY(enumerator.serializeDevice(-1, 0).isNull());
}

void TestAudioEnumerator::populate() {
    AudioEnumerator enumerator;

    for (int backend = 0; backend < enumerator.backends(); ++backend) {
        // check that there are no devices before populating
        QVERIFY(enumerator.devices(backend) == 1);

        enumerator.populate(backend);

        // check that there are 1 or more devices after populating
        // (the default device always exists even if the backend is not available)

        QVERIFY(enumerator.devices(backend) >= 1);
    }
}
