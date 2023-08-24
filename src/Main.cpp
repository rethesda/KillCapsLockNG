#include "Logging.h"

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

std::string asHexAddress(uint64_t decAddress) { return std::format("{:x}", decAddress); }

SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();
    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is starting...", plugin->GetName(), version);
    Init(skse);

    uint64_t baseAddress = (uint64_t)GetModuleHandleA("SkyrimSE.exe");
    bool gameIsAE = REL::Module::IsAE();
    bool gameIsVR = REL::Module::IsVR();

    std::string versionId = "";

    if (gameIsAE) { 
        versionId = "AE";
    } else if(gameIsVR) {
        versionId = "VR";
    } else {
        versionId = "SE";
    }

    log::info("SkyrimSE.exe ({}) - Base Address: {}", versionId, asHexAddress(baseAddress));

    // Prevent Capslock
    {

        // This means "xor al, al" - A way to zero a register (In this case the capslock flag - Ensuring that it is always set to FALSE). 
        // We also pad the instructions with one byte of noop (0x90) since the replacement is shorter than the original.
        uint_fast8_t assemblyHex[] = {0x30, 0xC0, 0x90}; 
        std::span<uint_fast8_t> instructions(assemblyHex);

        // First address identifier is SSE, second AE
        auto location = REL::RelocationID(67472, 68782).address(); // Get address of function: BSWin32KeyboardDevice::Process_140C1A130 (SSE)

        if (gameIsAE) {
            location += 0x250;  // (Original Instruction [AE: 140C53020] "setz al" replaced by "xor al, al")
        } else { // SSE and VR 
            location += 0x1BB;  
        }

        log::info(
            "Toggle capslock location - Replacing instructions at address: {} [{}]", 
            asHexAddress(location), 
            asHexAddress(location - baseAddress));

        REL::safe_write(location, instructions);
    }

    log::info("{} has finished.", plugin->GetName());
    return true;
}
