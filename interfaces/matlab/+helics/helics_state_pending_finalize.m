function v = helics_state_pending_finalize()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 105);
  end
  v = vInitialized;
end
