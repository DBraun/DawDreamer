#pragma once

#include "portable_endian.h"

class AbletonClipInfo {
    public:
        double loop_start;
        double loop_end;
        double start_marker;
        double hidden_loop_start;
        double hidden_loop_end;
        double end_marker;
        bool loop_on = true;
        bool warp_on = false;

        AbletonClipInfo() : loop_start(0), loop_end(262144), start_marker(0), hidden_loop_start(0), hidden_loop_end(262144), end_marker(262144), loop_on(true), warp_on(false) {

        }

        std::vector<std::pair<double, double>> warp_markers;

        int beat_to_sample(double beat, double sr) {

            double seconds;
            double bpm;
            beat_to_seconds(beat, seconds, bpm);
            return (int)(seconds * sr);
        }

        void beat_to_seconds(double beat, double& seconds, double&bpm) {

            if (warp_markers.size() < 2) {
                bpm = 120.;
                seconds = 60.*beat / bpm;
                return;
            }

            auto it = warp_markers.begin();

            double p1, b1, p2, b2;

            p1 = it->first;
            b1 = it->second;
            
            for (++it; it != warp_markers.end(); it++) {
                if (it->second >= beat) {

                    p2 = it->first;
                    b2 = it->second;

                    bpm = (b2 - b1) / (p2 - p1) * 60.0;
         
                    // interpolate between the two warp markers
                    float x = (beat - b1) / (b2 - b1);
                    
                    seconds = p1 + x * (p2 - p1);
                    return;
                }
                else {
                    p1 = it->first;
                    b1 = it->second;
                }
            }

            int last_index = warp_markers.size() - 1;
            p1 = warp_markers.at(last_index-1).first;
            b1 = warp_markers.at(last_index-1).second;
            p2 = warp_markers.at(last_index).first;
            b2 = warp_markers.at(last_index).second;

            bpm = (b2 - b1) / (p2 - p1) * 60.0;

            // interpolate between the two warp markers
            float x = (beat - b1) / (b2 - b1);

            seconds = p1 + x * (p2 - p1);
            return;
        }

        bool readWarpFile(const char* path) {

            reset();

            FILE* f = fopen(path, "rb");
            if (!f) {
                // Return because no warp file was found.
                throw std::runtime_error("Error: Couldn't open file at path: " + std::string(path));
            }

            if (!read_loop_info(f)) {
                fclose(f);
                throw std::runtime_error("Error: Couldn't find loop info in warp file: " + std::string(path));
            }
            else {
                //printf("loop_start: %.17g loop_end: %.17g start_marker: %.17g hidden_loop_start: %.17g hidden_loop_end: %.17g end_marker: %.17g\n",
                //    loop_start, loop_end, start_marker, hidden_loop_start, hidden_loop_end, end_marker);
            }
            rewind(f);

            double pos, beat;
            
            bool found_one = false;

            // the first appearance of "WarpMarker" isn't meaningful.
            find_str(f, "WarpMarker");

            long last_good_marker = 0;
            // Subsequent "WarpMarkers" are meaningful
            while (find_str(f, "WarpMarker")) {
                if (!fseek(f, 4, SEEK_CUR) &&
                    read_double(f, &pos) &&
                    read_double(f, &beat)) {

                    found_one = true;
                    warp_markers.push_back(std::make_pair(pos, beat));

                    last_good_marker = ftell(f);
                }
                else if (found_one) {
                    break;
                }
            }
            
            if (!fseek(f, last_good_marker, SEEK_SET) && !fseek(f, 7, SEEK_CUR) && read_bool(f, &loop_on)) {
                // Then we read the bool for loop_on
            }
            else {
                // Then we couldn't get to the byte for loop_on
                fclose(f);
                throw std::runtime_error("Error: Couldn't find loop on.");
            }

            fclose(f);
            return true;
        }
    private:

        void reset() {
            warp_on = false;
            warp_markers.clear();
        }

        int read_loop_info(FILE* f) {
            double sample_offset;

            // Assume it was saved with Ableton Live 10
            if (find_str(f, "SampleOverViewLevel") &&
                find_str(f, "SampleOverViewLevel") &&
                !fseek(f, 71, SEEK_CUR) &&
                read_double(f, &loop_start) &&
                read_double(f, &loop_end) &&
                read_double(f, &sample_offset) &&
                read_double(f, &hidden_loop_start) &&
                read_double(f, &hidden_loop_end) &&
                read_double(f, &end_marker) &&
                !fseek(f, 3, SEEK_CUR) &&
                read_bool(f, &warp_on)
                ) {
                start_marker = loop_start + sample_offset;
                return 1;
            }
            else {
                // Ableton Live 9?
                rewind(f);
                if (find_str(f, "SampleData") &&
                    find_str(f, "SampleData") &&
                    !fseek(f, 2702, SEEK_CUR) &&
                    read_double(f, &loop_start) &&
                    read_double(f, &loop_end) &&
                    read_double(f, &sample_offset) &&
                    read_double(f, &hidden_loop_start) &&
                    read_double(f, &hidden_loop_end) &&
                    read_double(f, &end_marker) &&
                    !fseek(f, 3, SEEK_CUR) &&
                    read_bool(f, &warp_on)
                    ) {
                    start_marker = loop_start + sample_offset;
                    return 1;
                }                
            }
            return 0;
        }

        int rot(int head, int size) {
            if (++head >= size) return head - size;
            return head;
        }
        int find_str(FILE* f, const char* string) {
            const int size = strlen(string);
            //char buffer[size];
            char* buffer = (char*)malloc(size * sizeof(char));
            if (fread(buffer, 1, size, f) != size) return 0;
            int head = 0, c;

            for (;;) {
                int rHead = head;
                for (int i = 0; i < size; i++) {
                    if (buffer[rHead] != string[i]) goto next;
                    rHead = rot(rHead, size);
                }
                return 1;

            next:
                if ((c = getc(f)) == EOF) break;
                buffer[head] = c;
                head = rot(head, size);
            }

            return 0;
        }

        int read_double(FILE* f, double* x) {
            if (fread(x, 1, 8, f) != 8) return 0;
            *(uint64_t*)x = le64toh(*(uint64_t*)x);
            return 1;
        }

        int read_bool(FILE* f, bool* b) {
            if (fread(b, 1, 1, f) != 1) return 0;
            return 1;
        }
};