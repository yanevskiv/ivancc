// (Test) Compiler error: [ERR_TXT_OPERAND_NOT_KNOWN]
// Can't read an operand of a kind the assembler does not know.

// Skipped: an operand reaches Txt_x86_64_Att_ReadOperand only as a field of
// an instruction pattern, which has matched it already.
#error "[ERR_TXT_OPERAND_NOT_KNOWN]"
