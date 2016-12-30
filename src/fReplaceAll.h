VOID fReplaceAll(std::basic_string<TCHAR> &sHayStack, std::basic_string<TCHAR> sNeedle, std::basic_string<TCHAR> sReplacement) {
  size_t uIndex = 0;
  while (uIndex < sNeedle.length()) {
    uIndex = sHayStack.find(sNeedle, uIndex);
    if (uIndex == std::basic_string<TCHAR>::npos) {
      break;
    }
    sHayStack.replace(uIndex, sNeedle.length(), sReplacement);
    uIndex += sReplacement.length(); 
  }
}
