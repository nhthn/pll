#include <algorithm>
#include <cmath>

class PLL {
public:
    PLL(float sampleRate)
        : m_sampleRate(sampleRate)
    {
        setLag(0.1);
    }

    enum class PhaseDetectorAlgorithm {
        Multiplier,
        JKFlipFlop,
        PFD
    };

    void setLag(float lag)
    {
        m_lpfK = std::exp(-1 / (lag * m_sampleRate));
    }

    void setVCOCenterFrequency(float vcoCenterFrequency)
    {
        m_vcoCenterFrequency = vcoCenterFrequency;
    }

    void setVCOFrequencyGain(float vcoFrequencyGain)
    {
        m_vcoFrequencyGain = vcoFrequencyGain;
    }

    void setPhaseDetectorAlgorithm(PhaseDetectorAlgorithm phaseDetectorAlgorithm)
    {
        m_phaseDetectorAlgorithm = phaseDetectorAlgorithm;
    }

    float process(float in)
    {
        float phaseDetectorOut;
        switch (m_phaseDetectorAlgorithm) {
        case PhaseDetectorAlgorithm::JKFlipFlop:
            if (m_inLast <= 0 && in > 0) {
                m_jkFlipFlopOut = 1;
            }
            if (m_vcoLast2 <= 0 && m_vcoLast > 0) {
                m_jkFlipFlopOut = -1;
            }
            phaseDetectorOut = m_jkFlipFlopOut;
            break;
        case PhaseDetectorAlgorithm::PFD:
            if (m_inLast <= 0 && in > 0) {
                m_pfdState = std::min(m_pfdState + 1, 1);
            }
            if (m_vcoLast2 <= 0 && m_vcoLast > 0) {
                m_pfdState = std::max(m_pfdState - 1, -1);
            }
            phaseDetectorOut = m_pfdState;
            break;
        default:
            phaseDetectorOut = m_vcoLast * in;
            break;
        }
        m_inLast = in;
        m_vcoLast2 = m_vcoLast;

        float lpfOut = phaseDetectorOut * (1 - m_lpfK) + m_lpfLast * m_lpfK;
        m_lpfLast = lpfOut;
        float vcoFrequency = m_vcoCenterFrequency + lpfOut * m_vcoFrequencyGain;
        float vcoPhaseIncrement = vcoFrequency / m_sampleRate;
        m_vcoPhase = std::fmod(m_vcoPhase + vcoPhaseIncrement, 1.0f);
        float vcoOut = m_vcoPhase < 0.5 ? 1 : -1;
        m_vcoLast = vcoOut;
        return vcoOut;
    }

private:
    const float m_sampleRate;

    float m_lpfK = 0;
    float m_vcoCenterFrequency = 440;
    float m_vcoFrequencyGain = 100;

    PhaseDetectorAlgorithm m_phaseDetectorAlgorithm = PhaseDetectorAlgorithm::Multiplier;

    float m_inLast = 0;
    float m_vcoLast2 = 0;
    int m_jkFlipFlopOut = 0;
    int m_pfdState = 0;

    float m_vcoLast = 0;
    float m_vcoPhase = 0;
    float m_lpfLast = 0;
};
