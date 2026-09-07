import http from 'k6/http';
import { check } from 'k6';
import { Counter } from 'k6/metrics';

const successfulBookings = new Counter('successful_bookings');
const seatConflicts = new Counter('seat_conflicts');

export const options = {
    scenarios: {
        booking_test: {
            executor: 'ramping-vus',

            startVUs: 0,

            stages: [
                { duration: '5s', target: 250 },
                { duration: '5s', target: 500 },
                { duration: '5s', target: 750 },
                { duration: '5s', target: 1000 },
                { duration: '10s', target: 1000 },
            ],

            gracefulRampDown: '2s',
        },
    },

    thresholds: {
        checks: ['rate==1.0'],
    },
};

export default function () {

    const seatNumber =
        ((__VU - 1) + __ITER) % 100 + 1;

    const response = http.post(
        `http://127.0.0.1:8080/book?seat=A${seatNumber}`
    );

    if (response.status === 201) {
        successfulBookings.add(1);
    }

    if (response.status === 409) {
        seatConflicts.add(1);
    }

    check(response, {
        'request handled correctly': (r) =>
            r.status === 201 || r.status === 409,
    });
}