/*
 * pedalboard
 * Copyright 2021 Spotify AB
 *
 * Licensed under the GNU Public License, Version 3.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    https://www.gnu.org/licenses/gpl-3.0.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

// This file was slightly modified from
// https://github.com/spotify/pedalboard/blob/49cefb490191679034238a14db6ce3cc7e96b79d/pedalboard/ExternalPlugin.h#L167-L245

class StandalonePluginWindow : public juce::DocumentWindow {
 public:
  StandalonePluginWindow(PluginProcessor& dawDreamerPluginProcessor,
                         juce::AudioProcessor& processor)
      : DocumentWindow("DawDreamer: " + processor.getName(),
                       juce::LookAndFeel::getDefaultLookAndFeel().findColour(
                           juce::ResizableWindow::backgroundColourId),
                       // juce::DocumentWindow::minimiseButton |
                       juce::DocumentWindow::closeButton),
        processor(processor),
        dawDreamerPluginProcessor(dawDreamerPluginProcessor) {
    setUsingNativeTitleBar(true);

    if (processor.hasEditor()) {
      if (auto* editor = processor.createEditorIfNeeded()) {
        setContentOwned(editor, true);
        setResizable(editor->isResizable(), false);
      } else {
        throw std::runtime_error("Failed to create plugin editor UI.");
      }
    } else {
      throw std::runtime_error("Plugin has no available editor UI.");
    }
  }

  /**
   * Open a native window to show a given AudioProcessor's editor UI,
   * pumping the juce::MessageManager run loop as necessary to service
   * UI events.
   */
  static void openWindowAndWait(PluginProcessor& dawDreamerPluginProcessor,
                                juce::AudioProcessor& processor) {
    bool shouldThrowErrorAlreadySet = false;

    JUCE_AUTORELEASEPOOL {
      StandalonePluginWindow window(dawDreamerPluginProcessor, processor);
      window.show();

      // Run in a tight loop so that we don't have to call ->stopDispatchLoop(),
      // which causes the MessageManager to become unusable in the future.
      // The window can be closed by sending a KeyboardInterrupt or closing
      // the window in the UI.
      while (window.isVisible()) {
        if (PyErr_CheckSignals() != 0) {
          window.closeButtonPressed();
          shouldThrowErrorAlreadySet = true;
          break;
        }

        {
          // Release the GIL to allow other Python threads to run in the
          // background while we the UI is running:
          py::gil_scoped_release release;
          juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
        }
      }
    }

    // Once the Autorelease pool has been drained, pump the dispatch loop one
    // more time to process any window close events:
    juce::MessageManager::getInstance()->runDispatchLoopUntil(10);

    if (shouldThrowErrorAlreadySet) {
      throw py::error_already_set();
    }
  }

  void closeButtonPressed() override {
    setVisible(false);

    for (int j = 0; j < 2; j++) {
      for (int i = 0; i < processor.getNumParameters(); ++i) {
        // give it a valid single sample of automation.
        dawDreamerPluginProcessor.setAutomationValByIndex(
            i, processor.getParameter(i));
      }
    }
  }

  ~StandalonePluginWindow() override { clearContentComponent(); }

  void show() {
    centreWithSize(getWidth(), getHeight());

    setVisible(true);
    toFront(true);
    juce::Process::makeForegroundProcess();
  }

 private:
  juce::AudioProcessor& processor;
  PluginProcessor& dawDreamerPluginProcessor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandalonePluginWindow)
};