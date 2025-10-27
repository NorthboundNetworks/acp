/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "ACP - Autonomous Command Protocol", "index.html", [
    [ "Autonomous Command Protocol v0.2 — Specification", "md_acp__comm__spec__v0-2.html", [
      [ "Purpose", "md_acp__comm__spec__v0-2.html#autotoc_md1", null ],
      [ "Goals", "md_acp__comm__spec__v0-2.html#autotoc_md3", null ],
      [ "Protocol Frame Structure (v0.2)", "md_acp__comm__spec__v0-2.html#autotoc_md5", null ],
      [ "Message Types", "md_acp__comm__spec__v0-2.html#autotoc_md7", [
        [ "GCS → Drone Commands", "md_acp__comm__spec__v0-2.html#autotoc_md8", null ],
        [ "Drone → GCS Telemetry / Events", "md_acp__comm__spec__v0-2.html#autotoc_md9", null ],
        [ "Responses", "md_acp__comm__spec__v0-2.html#autotoc_md10", null ],
        [ "Response Structs", "md_acp__comm__spec__v0-2.html#autotoc_md11", null ]
      ] ],
      [ "Example Structs (Packed C Representations)", "md_acp__comm__spec__v0-2.html#autotoc_md13", [
        [ "<span class=\"tt\">cmd_control_t</span>", "md_acp__comm__spec__v0-2.html#autotoc_md14", null ],
        [ "<span class=\"tt\">tlm_status_t</span>", "md_acp__comm__spec__v0-2.html#autotoc_md15", null ],
        [ "<span class=\"tt\">tlm_audit_t</span>", "md_acp__comm__spec__v0-2.html#autotoc_md16", null ]
      ] ],
      [ "Safety &amp; Mediation Requirements", "md_acp__comm__spec__v0-2.html#autotoc_md18", [
        [ "Error Codes", "md_acp__comm__spec__v0-2.html#autotoc_md19", null ]
      ] ],
      [ "Extensibility &amp; Future Features", "md_acp__comm__spec__v0-2.html#autotoc_md21", null ],
      [ "Implementation Notes", "md_acp__comm__spec__v0-2.html#autotoc_md23", null ],
      [ "Success Criteria", "md_acp__comm__spec__v0-2.html#autotoc_md25", null ],
      [ "Out of Scope", "md_acp__comm__spec__v0-2.html#autotoc_md27", null ]
    ] ],
    [ "Autonomous Command Protocol (ACP) v0.3 — Complete Specification", "md_acp__comm__spec__v0-3.html", [
      [ "Executive Summary", "md_acp__comm__spec__v0-3.html#autotoc_md29", null ],
      [ "Design Principles", "md_acp__comm__spec__v0-3.html#autotoc_md30", null ],
      [ "Key Features", "md_acp__comm__spec__v0-3.html#autotoc_md31", null ],
      [ "Protocol Architecture", "md_acp__comm__spec__v0-3.html#autotoc_md32", [
        [ "Wire Format", "md_acp__comm__spec__v0-3.html#autotoc_md33", null ],
        [ "COBS Encoding Properties", "md_acp__comm__spec__v0-3.html#autotoc_md34", null ],
        [ "CRC16 Implementation", "md_acp__comm__spec__v0-3.html#autotoc_md35", null ],
        [ "Frame Types", "md_acp__comm__spec__v0-3.html#autotoc_md36", [
          [ "Telemetry Frames", "md_acp__comm__spec__v0-3.html#autotoc_md37", null ],
          [ "Command Frames", "md_acp__comm__spec__v0-3.html#autotoc_md38", null ]
        ] ],
        [ "Session Management", "md_acp__comm__spec__v0-3.html#autotoc_md39", null ]
      ] ],
      [ "Implementation Guidelines", "md_acp__comm__spec__v0-3.html#autotoc_md40", [
        [ "Payload Format", "md_acp__comm__spec__v0-3.html#autotoc_md41", null ],
        [ "Error Handling", "md_acp__comm__spec__v0-3.html#autotoc_md42", null ],
        [ "Performance Considerations", "md_acp__comm__spec__v0-3.html#autotoc_md43", null ]
      ] ],
      [ "Security Model", "md_acp__comm__spec__v0-3.html#autotoc_md44", [
        [ "Authentication Architecture", "md_acp__comm__spec__v0-3.html#autotoc_md45", null ],
        [ "Cryptographic Specifications", "md_acp__comm__spec__v0-3.html#autotoc_md46", null ],
        [ "Threat Model", "md_acp__comm__spec__v0-3.html#autotoc_md47", null ]
      ] ],
      [ "API Reference", "md_acp__comm__spec__v0-3.html#autotoc_md48", [
        [ "Core Functions", "md_acp__comm__spec__v0-3.html#autotoc_md49", null ],
        [ "Testing", "md_acp__comm__spec__v0-3.html#autotoc_md50", null ],
        [ "Build Requirements", "md_acp__comm__spec__v0-3.html#autotoc_md51", null ]
      ] ],
      [ "Conclusion", "md_acp__comm__spec__v0-3.html#autotoc_md52", null ]
    ] ],
    [ "ACP Library Build Guide", "md__b_u_i_l_d.html", [
      [ "Table of Contents", "md__b_u_i_l_d.html#autotoc_md54", null ],
      [ "Quick Start", "md__b_u_i_l_d.html#autotoc_md55", [
        [ "Prerequisites", "md__b_u_i_l_d.html#autotoc_md56", null ],
        [ "Fast Build (CMake)", "md__b_u_i_l_d.html#autotoc_md57", null ],
        [ "Fast Build (Make)", "md__b_u_i_l_d.html#autotoc_md58", null ]
      ] ],
      [ "Build Systems", "md__b_u_i_l_d.html#autotoc_md59", [
        [ "CMake (Recommended)", "md__b_u_i_l_d.html#autotoc_md60", null ],
        [ "Make", "md__b_u_i_l_d.html#autotoc_md61", null ]
      ] ],
      [ "Platform Support", "md__b_u_i_l_d.html#autotoc_md62", [
        [ "Linux (x86_64, ARM64)", "md__b_u_i_l_d.html#autotoc_md63", null ],
        [ "macOS (Intel, Apple Silicon)", "md__b_u_i_l_d.html#autotoc_md64", null ],
        [ "Windows (MinGW, MSVC)", "md__b_u_i_l_d.html#autotoc_md65", null ],
        [ "Embedded Systems", "md__b_u_i_l_d.html#autotoc_md66", null ]
      ] ],
      [ "Build Options", "md__b_u_i_l_d.html#autotoc_md67", [
        [ "CMake Options", "md__b_u_i_l_d.html#autotoc_md68", null ],
        [ "Make Options", "md__b_u_i_l_d.html#autotoc_md69", null ]
      ] ],
      [ "Installation", "md__b_u_i_l_d.html#autotoc_md70", [
        [ "System-wide Installation", "md__b_u_i_l_d.html#autotoc_md71", null ],
        [ "Package Manager Integration", "md__b_u_i_l_d.html#autotoc_md72", null ]
      ] ],
      [ "Integration", "md__b_u_i_l_d.html#autotoc_md73", [
        [ "Using Static Library", "md__b_u_i_l_d.html#autotoc_md74", null ],
        [ "Using Shared Library", "md__b_u_i_l_d.html#autotoc_md75", null ],
        [ "Code Example", "md__b_u_i_l_d.html#autotoc_md76", null ]
      ] ],
      [ "Cross-Compilation", "md__b_u_i_l_d.html#autotoc_md77", [
        [ "ARM Linux (aarch64)", "md__b_u_i_l_d.html#autotoc_md78", null ],
        [ "ARM Embedded (Cortex-M)", "md__b_u_i_l_d.html#autotoc_md79", null ],
        [ "Windows from Linux (MinGW)", "md__b_u_i_l_d.html#autotoc_md80", null ]
      ] ],
      [ "Troubleshooting", "md__b_u_i_l_d.html#autotoc_md81", [
        [ "Common Build Issues", "md__b_u_i_l_d.html#autotoc_md82", null ],
        [ "Platform-Specific Issues", "md__b_u_i_l_d.html#autotoc_md83", null ],
        [ "Debug Builds", "md__b_u_i_l_d.html#autotoc_md84", null ],
        [ "Getting Help", "md__b_u_i_l_d.html#autotoc_md85", null ]
      ] ]
    ] ],
    [ "ACP CRC16-CCITT Implementation", "md_crc16.html", [
      [ "Overview", "md_crc16.html#autotoc_md88", null ],
      [ "Polynomial and Parameters", "md_crc16.html#autotoc_md89", null ],
      [ "Implementation Details", "md_crc16.html#autotoc_md90", null ],
      [ "Test Vectors", "md_crc16.html#autotoc_md91", [
        [ "Standard Test Vectors", "md_crc16.html#autotoc_md92", null ],
        [ "ACP-Specific Test Cases", "md_crc16.html#autotoc_md93", null ],
        [ "Frame-Level Validation", "md_crc16.html#autotoc_md94", null ]
      ] ],
      [ "Verification Methods", "md_crc16.html#autotoc_md95", [
        [ "Self-Test Function", "md_crc16.html#autotoc_md96", null ],
        [ "Manual Verification", "md_crc16.html#autotoc_md97", null ]
      ] ],
      [ "Integration with ACP Protocol", "md_crc16.html#autotoc_md98", [
        [ "Frame Processing Flow", "md_crc16.html#autotoc_md99", null ],
        [ "Error Detection Capability", "md_crc16.html#autotoc_md100", null ]
      ] ],
      [ "Standards Compliance", "md_crc16.html#autotoc_md101", null ],
      [ "Performance Characteristics", "md_crc16.html#autotoc_md102", null ],
      [ "References", "md_crc16.html#autotoc_md103", null ],
      [ "Implementation Validation", "md_crc16.html#autotoc_md104", null ]
    ] ],
    [ "ACP Keystore Implementation and Management", "md_keystore.html", [
      [ "Overview", "md_keystore.html#autotoc_md106", null ],
      [ "Keystore Architecture", "md_keystore.html#autotoc_md107", [
        [ "Design Principles", "md_keystore.html#autotoc_md108", null ],
        [ "Default Implementation", "md_keystore.html#autotoc_md109", null ]
      ] ],
      [ "File Format Specification", "md_keystore.html#autotoc_md110", [
        [ "Keystore File Structure", "md_keystore.html#autotoc_md111", null ],
        [ "File Format Details", "md_keystore.html#autotoc_md112", null ],
        [ "Example Keystore File", "md_keystore.html#autotoc_md113", null ]
      ] ],
      [ "Key Management Operations", "md_keystore.html#autotoc_md114", [
        [ "Loading Keys", "md_keystore.html#autotoc_md115", null ],
        [ "Adding New Keys", "md_keystore.html#autotoc_md116", null ],
        [ "Key Rotation Procedure", "md_keystore.html#autotoc_md117", null ]
      ] ],
      [ "Security Considerations", "md_keystore.html#autotoc_md118", [
        [ "File Permissions", "md_keystore.html#autotoc_md119", null ],
        [ "Key Generation", "md_keystore.html#autotoc_md120", null ],
        [ "Key Storage Best Practices", "md_keystore.html#autotoc_md121", null ]
      ] ],
      [ "Platform-Specific Integration", "md_keystore.html#autotoc_md122", [
        [ "Custom Keystore Backend", "md_keystore.html#autotoc_md123", null ],
        [ "Embedded Systems Integration", "md_keystore.html#autotoc_md124", null ]
      ] ],
      [ "Troubleshooting", "md_keystore.html#autotoc_md125", [
        [ "Common Issues", "md_keystore.html#autotoc_md126", null ],
        [ "Diagnostic Commands", "md_keystore.html#autotoc_md127", null ],
        [ "Recovery Procedures", "md_keystore.html#autotoc_md128", null ]
      ] ],
      [ "Integration Examples", "md_keystore.html#autotoc_md129", [
        [ "Production Deployment", "md_keystore.html#autotoc_md130", null ],
        [ "Development/Testing", "md_keystore.html#autotoc_md131", null ]
      ] ],
      [ "References", "md_keystore.html#autotoc_md132", null ]
    ] ],
    [ "Specification Quality Checklist: ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html", [
      [ "Content Quality", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md161", null ],
      [ "Requirement Completeness", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md162", null ],
      [ "Feature Readiness", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md163", null ],
      [ "Notes", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md164", null ],
      [ "Validation Results (Iteration 1)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md166", null ],
      [ "Validation Results (Iteration 2)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md168", null ]
    ] ],
    [ "C API Contract — ACP v0.3", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html", [
      [ "Conventions", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md170", null ],
      [ "Types (conceptual)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md171", null ],
      [ "Functions", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md172", [
        [ "Framing and Integrity", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md173", null ],
        [ "Session Management", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md174", null ],
        [ "Keystore Interface (platform shim)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md175", null ]
      ] ],
      [ "Error Semantics", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md176", null ],
      [ "Versioning", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md177", null ]
    ] ],
    [ "Data Model — ACP v0.3", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html", [
      [ "Entities", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md179", [
        [ "ACP Frame", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md180", null ],
        [ "Session", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md181", null ],
        [ "Keystore", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md182", null ],
        [ "Platform Shims", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md183", null ]
      ] ],
      [ "State Transitions (Session)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md184", null ],
      [ "Error Codes (excerpt)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md185", null ]
    ] ],
    [ "Implementation Plan: ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html", [
      [ "Summary", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md187", null ],
      [ "Technical Context", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md188", null ],
      [ "Constitution Check", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md189", null ],
      [ "Project Structure", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md190", [
        [ "Documentation (this feature)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md191", null ],
        [ "Source Code (repository root)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md192", null ]
      ] ],
      [ "Complexity Tracking", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md193", null ]
    ] ],
    [ "Quickstart — ACP Reference C Library", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html", [
      [ "Prerequisites", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md195", null ],
      [ "Build", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md196", null ],
      [ "Minimal usage", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md197", null ],
      [ "Tests", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md198", null ],
      [ "Platform shims", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md199", null ],
      [ "Notes", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md200", null ]
    ] ],
    [ "Phase 0 Research — ACP Protocol Implementation Details", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html", [
      [ "Framing and Integrity", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md202", null ],
      [ "Authentication and Replay Protection", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md203", null ],
      [ "Frame Header and Endianness", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md204", null ],
      [ "Payload Constraints and Errors", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md205", null ],
      [ "Keystore", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md206", null ],
      [ "Build and Toolchains", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md207", null ],
      [ "Testing", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md208", null ],
      [ "Open Questions (resolved here)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md209", null ]
    ] ],
    [ "Feature Specification: ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html", [
      [ "Overview", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md211", null ],
      [ "Clarifications", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md212", [
        [ "Session 2025-10-27", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md213", null ]
      ] ],
      [ "User Scenarios &amp; Testing (mandatory)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md214", [
        [ "User Story 1 - Encode/Decode Telemetry Frame (Priority: P1)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md215", null ],
        [ "User Story 2 - Authenticate Session With HMAC (Priority: P1)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md217", null ],
        [ "User Story 3 - Cross-Platform Build and Link (Priority: P2)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md219", null ],
        [ "User Story 4 - Platform Abstraction Fallbacks (Priority: P3)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md221", null ],
        [ "Edge Cases", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md222", null ]
      ] ],
      [ "Requirements (mandatory)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md223", [
        [ "Functional Requirements", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md224", null ],
        [ "Key Entities (data-oriented)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md225", null ]
      ] ],
      [ "Success Criteria (mandatory)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md226", [
        [ "Measurable Outcomes", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md227", null ]
      ] ],
      [ "Assumptions and Dependencies", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md228", null ]
    ] ],
    [ "Implementation Tasks — ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html", [
      [ "Phase 1 — Setup (project initialization)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md230", null ],
      [ "Phase 2 — Foundational (blocking prerequisites)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md231", null ],
      [ "Phase 3 — User Story 1 (P1): Encode/Decode Telemetry Frame", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md232", null ],
      [ "Phase 4 — User Story 2 (P1): Authenticate Session With HMAC", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md233", null ],
      [ "Phase 5 — User Story 3 (P2): Cross-Platform Build and Link", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md234", null ],
      [ "Phase 5A — Extended Build System (completed)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md235", null ],
      [ "Phase 6 — User Story 4 (P3): Platform Abstraction Fallbacks", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md236", null ],
      [ "Final Phase — Polish &amp; Cross-Cutting", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md237", null ],
      [ "Dependencies (story completion order)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md238", null ],
      [ "Parallel execution examples", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md239", null ],
      [ "Implementation Status Update", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md240", null ],
      [ "Implementation Summary (October 27, 2025)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md241", [
        [ "<b>Core Protocol Features (100% Complete)</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md242", null ],
        [ "<b>Build System &amp; Developer Experience (100% Complete)</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md243", null ],
        [ "<b>Testing &amp; Validation (100% Complete)</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md244", null ],
        [ "<b>Production Readiness Criteria Met</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md245", null ],
        [ "Priority Implementation Complete (October 27, 2025)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md246", null ]
      ] ],
      [ "Implementation strategy", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md247", null ],
      [ "Validation", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md248", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"_c_make_c_compiler_id_8c_source.html",
"acp__platform__posix_8c.html#a2c83600265e90db7a27efb252def9f06",
"md__b_u_i_l_d.html#autotoc_md67"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';