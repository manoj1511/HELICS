function v = helics_error_insufficient_space()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 57);
  end
  v = vInitialized;
end
