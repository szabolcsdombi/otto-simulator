import React from 'react';
import { Demo } from './demo';

export const Home = () => {
  return (
    <div
      style={{
        width: 'min(800px, 100%)',
        margin: 'auto',
      }}
    >
      <Demo size={800} resolution={1600} />
    </div>
  );
};
